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
#include "Exception.hpp"
#include <sstream>

const std::ledger_exp::optional<ledger::core::api::Error> ledger::core::Exception::NO_CORE_ERROR;

ledger::core::Exception::Exception(api::ErrorCode code, const std::string &message, Option<std::shared_ptr<void>> userData) {
    _code = code;
    _message = message;
    _userData = userData;
}

ledger::core::Exception::~Exception() {

}

const char *ledger::core::Exception::what() const noexcept {
    return _message.c_str();
}

ledger::core::api::ErrorCode ledger::core::Exception::getErrorCode() const {
    return _code;
}

ledger::core::api::Error ledger::core::Exception::toApiError() const {
    return api::Error(_code, _message);
}

std::string ledger::core::Exception::getMessage() const {
    return fmt::format("{} ({})", _message, api::to_string(_code));
}

const ledger::core::Option<std::shared_ptr<void>> &ledger::core::Exception::getUserData() const {
    return _userData;
}
