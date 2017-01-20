/*
 *
 * Future
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/01/2017.
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
#ifndef LEDGER_CORE_FUTURE_HPP
#define LEDGER_CORE_FUTURE_HPP

#include <exception>
#include <memory>
#include <functional>
#include <list>
#include "Deffered.hpp"
#include "../api/ExecutionContext.hpp"
#include "ExecutionContext.hpp"

namespace ledger {
    namespace core {

        template <typename T>
        class Future {

        public:
            Future(const std::shared_ptr<Deffered<T>> &_defer) : _defer(_defer) {}
            Future(const Future<T>& future) {
                _defer = future._defer;
            }
            Future(Future<T>&& future) : _defer(std::move(future._defer)) {}
            Future<T>& operator=(const Future<T>& future) {
                if (this != &future)
                    _defer = future._defer;
            }
            Future<T>& operator=(Future<T>&& future) {
                if (this != &future)
                    _defer = std::move(future._defer);
            }

            template <typename R>
            Future<R> map(const std::shared_ptr<ExecutionContext>& context, std::function<R (const T&)> map);

        private:
            std::shared_ptr<Deffered<T>> _defer;
        };

    }
}


#endif //LEDGER_CORE_FUTURE_HPP
