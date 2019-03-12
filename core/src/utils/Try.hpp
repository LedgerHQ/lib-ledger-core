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

#ifdef TARGET_JNI
#include <jni/jni/djinni_support.hpp>
#endif

#include <functional>
#include "optional.hpp"
#include "Option.hpp"
#include "Exception.hpp"
#include <typeinfo>
#include <stdexcept>
#include <boost/exception/diagnostic_information.hpp>

namespace ledger {
    namespace core {
        template <typename T>
        class Try {
        public:
            Try(const T&v) {
                _value = optional<T>(v);
            }

            Try(api::ErrorCode code, const std::string& message) {
                _exception = Exception(code, message);
            }

            Try(const Exception& ex) {
                _exception = ex;
            }

            const T& getValue() const {
                return _value.value();
            }

            const Exception& getFailure() const {
                return _exception.value();
            }

            bool isFailure() const {
                return _exception ? true : false;
            }
            bool isSuccess() const {
                return _value ? true : false;
            }

            Option<T> toOption() const {
                if (isSuccess())
                    return Option<T>(getValue());
                return Option<T>();
            }

            Option<Exception> exception() const {
                if (isFailure())
                    return Option<Exception>(getFailure());
                return Option<Exception>();
            }

            const T& getOrThrow() const {
                if (isFailure())
                    throw getFailure();
                return getValue();
            }

            template <class U>
            const T& getOrThrowException() const {
                if (isFailure())
                    throw U(getFailure().getMessage());
                return getValue();
            }

            Try<T> mapException(api::ErrorCode code) {
                if (isFailure())
                    return Try<T>(code, getFailure().getMessage());
                return *this;
            }

        private:
            optional<Exception> _exception;
            optional<T> _value;

        public:
            static const Try<T> from(std::function<T ()> lambda) {
                try {
                    return Try<T>(lambda());
                } catch (const Exception& ex) {
                    return Try<T>(ex);
#ifdef TARGET_JNI
                } catch (const djinni::jni_exception& ex) {
                    return Try<T>(api::ErrorCode::RUNTIME_ERROR, ex.get_backtrace());
#endif
                } catch (...) {
                    return Try<T>(api::ErrorCode::RUNTIME_ERROR, boost::current_exception_diagnostic_information(true));
                }
            }
        };

        template <typename T>
        using TryPtr = Try<std::shared_ptr<T>>;

        template<typename T>
        Try<T> make_try(std::function<T ()> lambda) {
            return Try<T>::from(lambda);
        }
    }
}


#endif //LEDGER_CORE_TRY_HPP
