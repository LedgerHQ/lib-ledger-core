/*
 * Assert
 *
 * Created by Hakim Aammar on 30/07/2020
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#ifndef LEDGER_CORE_ASSERT_H
#define LEDGER_CORE_ASSERT_H

#include <utils/Exception.hpp>
#include <collections/vector.hpp>
#include <api/ErrorCode.hpp>
#include <string>

namespace ledger {
    namespace core {

        const void assertCurrency(const bool isCurrency, const std::string &errorMessage);

        template <typename T>
        const void assertSingleRow(const std::vector<T> rows, const std::string & msg) {
            if (rows.size() != 1) {
                throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, msg);
            }
        };

    }
}

#endif
