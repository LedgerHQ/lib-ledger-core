/*
 *
 * Promise
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
#ifndef LEDGER_CORE_PROMISE_HPP
#define LEDGER_CORE_PROMISE_HPP

#include "../utils/Try.hpp"
#include "../utils/Option.hpp"
#include "../utils/Exception.hpp"
#include "Future.hpp"

namespace ledger {
    namespace core {

        template <typename T>
        class Promise {
        public:
            Promise() {
                _deffer = std::make_shared<Deffered<T>>();
            };
            Promise(const T& value) : Promise() {
                success(value);
            };

            Promise(const Exception& failure) : Promise() {
                this->failure(failure);
            };

            void complete(const Try<T>& result) {
                _deffer->setResult(result);
            };

            bool tryComplete(const Try<T>& result) {
               return Try<Unit>::from([result, this] () {
                   complete(result);
                   return unit;
               }).isSuccess();
            }

            void completeWith(const Future<T>& future) {
                auto deffer = _deffer;
                Future<T> f = future;
                f.onComplete(ImmediateExecutionContext::INSTANCE, [deffer] (const Try<T>& result) {
                    deffer->setResult(result);
                });
            };

            void tryCompleteWith(const Future<T>& future) {
                auto deffer = _deffer;
                Future<T> f = future;
                f.onComplete(ImmediateExecutionContext::INSTANCE, [deffer] (const Try<T>& result) {
                    Try<Unit>::from([result, deffer] () {
                        deffer->setResult(result);
                        return unit;
                    });
                });
            }

            bool trySuccess(const T& value) {
                return Try<Unit>::from([value, this] () {
                    success(value);
                    return unit;
                }).isSuccess();
            }

            bool tryFailure(const Exception& exception) {
                return Try<Unit>::from([exception, this] () {
                    failure(exception);
                    return unit;
                }).isSuccess();
            }

            void success(const T& value) {
                _deffer->setValue(value);
            };

            void failure(const Exception& exception) {
                _deffer->setError(exception);
            };

            bool isCompleted() const {
                return _deffer->hasValue();
            };

            Future<T> getFuture() {
                return Future<T>(_deffer);
            };

        private:
            std::shared_ptr<Deffered<T>> _deffer;
        };
    }
}


#endif //LEDGER_CORE_PROMISE_HPP
