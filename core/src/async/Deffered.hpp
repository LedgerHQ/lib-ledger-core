/*
 *
 * Deffered
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/01/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#ifndef LEDGER_CORE_DEFFERED_HPP
#define LEDGER_CORE_DEFFERED_HPP

#include <memory>
#include <functional>
#include "../utils/Option.hpp"
#include "../utils/Try.hpp"
#include <mutex>
#include <src/utils/Exception.hpp>
#include <queue>
#include "../api/ExecutionContext.hpp"
#include <tuple>
#include "../utils/LambdaRunnable.hpp"

namespace ledger {
    namespace core {

        template <typename T>
        class Future;

        template <typename T>
        class Promise;

        template <typename T>
        class Deffered {

        public:
            using Callback = std::function<void (const Try<T>&)>;

            friend class Future<T>;
            friend class Promise<T>;
            Deffered() {

            };
            Deffered(const Deffered&) = delete;
            Deffered(Deffered&&) = delete;

            void setResult(const Try<T>& result) {
                {
                    std::lock_guard<std::mutex> lock(_lock);
                    _value = result;
                }
                trigger();
            }

            void setValue(T& value) {
                {
                    std::lock_guard<std::mutex> lock(_lock);
                    _value = Try<T>(value);
                }
                trigger();
            };

            void setError(const Exception& exception) {
                {
                    std::lock_guard<std::mutex> lock(_lock);
                    Try<T> ex;
                    ex.fail(exception);
                    _value = ex;
                }
                trigger();
            }

            void addCallback(Callback callback, std::shared_ptr<api::ExecutionContext> context) {
                // Add to the queue
                {
                    std::lock_guard<std::mutex> lock(_lock);
                    _callbacks.push(std::make_tuple(callback, context));
                }
                trigger();
            }

            Option<Try<T>> getValue() const {
                std::lock_guard<std::mutex> lock(_lock);
                Option<Try<T>> cpy = _value;
                return cpy;
            }


            void trigger() {
                std::lock_guard<std::mutex> lock(_lock);
                if (_value.isEmpty() || _callbacks.empty()) {
                    return ;
                }
                while (!_callbacks.empty()) {
                    std::tuple<Callback, std::shared_ptr<api::ExecutionContext>> callback = _callbacks.front();
                    Callback cb = std::get<0>(callback);
                    auto value = _value.getValue();
                    std::get<1>(callback)->execute(make_runnable([cb, value] () {
                        cb(value);
                    }));
                    _callbacks.pop();
                }
            }

        private:
            std::mutex _lock;
            Option<Try<T>> _value;
            std::queue<std::tuple<Callback, std::shared_ptr<api::ExecutionContext>>> _callbacks;
        };



    }
}


#endif //LEDGER_CORE_DEFFERED_HPP
