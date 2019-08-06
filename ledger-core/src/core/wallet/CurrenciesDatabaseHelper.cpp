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

#include <core/wallet/CurrenciesDatabaseHelper.hpp>
#include <core/utils/hex.h>
#include <core/api/enum_from_string.hpp>
#include <core/api/Currency.hpp>
#include <core/collections/strings.hpp>

using namespace soci;

bool ledger::core::CurrenciesDatabaseHelper::insertCurrency(
    soci::session &sql,
    const ledger::core::api::Currency &currency
) {
    int count;
    sql << "SELECT COUNT(*) FROM currencies WHERE name = :name",
        soci::use(currency.name),
        soci::into(count);

    bool inserted = false;

    if (count == 0) {
        // Insert currency
        sql << "INSERT INTO currencies VALUES(:name, :bip44, :uri)",
            use(currency.name),
            use(currency.bip44CoinType),
            use(currency.paymentUriScheme);

        inserted = true;
    }

    insertUnits(sql, currency);
    return inserted;
}

void ledger::core::CurrenciesDatabaseHelper::getAllCurrencies(soci::session &sql,
                                                              std::vector<ledger::core::api::Currency> &currencies) {
    rowset<row> rows = (sql.prepare << "SELECT currencies.name, currencies.bip44_coin_type, currencies.payment_uri_scheme FROM currencies ");
    for (auto& currency_row : rows) {
        auto offset = 0;
        api::Currency currency;
        currency.name = currency_row.get<std::string>(0);
        currency.bip44CoinType = currency_row.get<int32_t>(1);
        currency.paymentUriScheme = currency_row.get_indicator(2) == i_null ? "" : currency_row.get<std::string>(2);

        getAllUnits(sql, currency);
        currencies.push_back(currency);
    }
}

void ledger::core::CurrenciesDatabaseHelper::getAllUnits(soci::session &sql, ledger::core::api::Currency &currency) {
    rowset<row> rows = (sql.prepare <<
        "SELECT name, magnitude, code FROM units WHERE currency_name = :currency",
        use(currency.name)
    );

    for (auto& row : rows) {
        api::CurrencyUnit unit;
        unit.name = row.get<std::string>(0);
        unit.numberOfDecimal = row.get<int32_t>(1);
        unit.code = row.get<std::string>(2);
        currency.units.push_back(unit);
    }
}

void ledger::core::CurrenciesDatabaseHelper::insertUnits(soci::session &sql, const ledger::core::api::Currency &currency) {
    for (const auto& unit : currency.units) {
        int count;
        sql << "SELECT COUNT(*) FROM units WHERE name = :name AND currency_name = :cname", use(unit.name), use(currency.name), into(count);
        if (count == 0) {
            sql << "INSERT INTO units VALUES(:name, :magnitude, :code, :currency_name)",
            use(unit.name), use(unit.numberOfDecimal), use(unit.code), use(currency.name);
        }
    }
}

void ledger::core::CurrenciesDatabaseHelper::removeCurrency(soci::session &sql, const std::string &currencyName) {
    sql << "DELETE FROM currencies WHERE name = :name", use(currencyName);
}
