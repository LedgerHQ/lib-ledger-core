/*
 *
 * CurrenciesDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/05/2017.
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
#ifndef LEDGER_CORE_CURRENCIESDATABASEHELPER_HPP
#define LEDGER_CORE_CURRENCIESDATABASEHELPER_HPP

#include <soci.h>
#include <api/Currency.hpp>
#include <api/ERC20Token.hpp>

namespace ledger {
    namespace core {
        class CurrenciesDatabaseHelper {
        public:
            static bool insertCurrency(soci::session& sql, const api::Currency& currency);
            static bool insertERC20Token(soci::session &sql,
                                         const ledger::core::api::ERC20Token &token);
            static void getAllCurrencies(soci::session& sql, std::vector<api::Currency>& currencies);
            static void insertUnits(soci::session& sql, const api::Currency& currency);
            static void getAllUnits(soci::session& sql, api::Currency& currency);
            static void removeCurrency(soci::session& sql, const std::string& currencyName);
        };
    }
}


#endif //LEDGER_CORE_CURRENCIESDATABASEHELPER_HPP
