/*
 *
 * Try
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/12/2016.
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
#ifndef LEDGER_CORE_TRY_HPP
#define LEDGER_CORE_TRY_HPP

#include <functional>
#include "optional.hpp"

namespace ledger {
    namespace core {
        template <typename T>
        class Try {
        public:
            Try() {

            };

            void fail(const std::exception &exception) {
                _exception = exception;
            }

            void success(const T& v) {
                _value = v;
            }

            const T& getValue() const {
                return _value.value();
            }

            const std::exception& getFailure() const {
                return _exception.value();
            }

            bool isFailure() const {
                return _exception ? true : false;
            }
            bool isSuccess() const {
                return _value ? true : false;
            }
            bool isComplete() const {
                return isSuccess() || isFailure();
            }

            ~Try() {

            };
        private:
            optional<std::exception> _exception;
            optional<T> _value;

        public:
            static const Try<T> tryAndCatch(std::function<T ()> lambda) {
                Try<T> result;
                try {
                    result.success(lambda());
                } catch (const std::exception& ex) {
                    result.fail(ex);
                }
                return result;
            }
        };
    }
}


#endif //LEDGER_CORE_TRY_HPP
