/*
 *
 * Exception
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
#ifndef LEDGER_CORE_EXCEPTION_HPP
#define LEDGER_CORE_EXCEPTION_HPP

#include <exception>
#include <string>
#include "../api/ErrorCode.hpp"
#include "../api/Error.hpp"
#include "../utils/Option.hpp"
#include <memory>
#include <fmt/format.h>
#include <iostream>

namespace ledger {
    namespace core {
        class Exception : public std::exception {
        public:
            Exception(api::ErrorCode code, const std::string& message,
                      Option<std::shared_ptr<void>> userData = Option<std::shared_ptr<void>>());
            api::ErrorCode getErrorCode() const;
            std::string getMessage() const;
            const Option<std::shared_ptr<void>>& getUserData() const;

            template<typename T> Option<std::shared_ptr<T>> getTypeUserData() const {
                return _userData.map<std::shared_ptr<T>>([] (const std::shared_ptr<void>& ptr) -> std::shared_ptr<T> {
                    return std::static_pointer_cast<T>(ptr);
                });
            };

            api::Error toApiError() const;

            virtual ~Exception() override;

            virtual const char *what() const noexcept override;

            friend std::ostream &operator<<(std::ostream &os, const Exception &e) {
                return os << "Exception(" << e.getMessage() << ")";
            }

        public:
            static const optional<api::Error> NO_ERROR;

        private:
            api::ErrorCode _code;
            std::string _message;
            Option<std::shared_ptr<void>> _userData;
        };

        template <typename... Args>
        Exception make_exception(api::ErrorCode code, const std::string& format, const Args&... args) {
            return Exception(code, fmt::format(format, args...));
        };

        template <typename... Args>
        Exception make_exception(api::ErrorCode code, std::shared_ptr<void> userData, const std::string& format, const Args&... args) {
            return Exception(code, fmt::format(format, args...), userData);
        };
    }
}


#endif //LEDGER_CORE_EXCEPTION_HPP
