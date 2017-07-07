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
#include "CurrenciesDatabaseHelper.hpp"
#include <api/BitcoinLikeNetworkParameters.hpp>
#include <utils/hex.h>
#include <api/enum_from_string.hpp>
#include <api/Currency.hpp>
#include <api/BitcoinLikeFeePolicy.hpp>

using namespace soci;

bool ledger::core::CurrenciesDatabaseHelper::insertCurrency(soci::session &sql,
                                                            const ledger::core::api::Currency &currency) {
    int count;
    sql << "SELECT COUNT(*) FROM currencies WHERE name = :name",
    soci::use(currency.name),
    soci::into(count);
    bool inserted = false;
    if (count == 0) {
        // Insert currency
        sql << "INSERT INTO currencies VALUES(:name, :type, :bip44, :uri)",
            use(currency.name),
            use(api::to_string(currency.walletType)),
            use(currency.bip44CoinType),
            use(currency.paymentUriScheme);

        // Insert network parameters
        switch (currency.walletType) {
            case api::WalletType::BITCOIN: {
                auto &params = currency.bitcoinLikeNetworkParameters.value();
                sql
                << "INSERT INTO bitcoin_currencies VALUES(:name, :p2pkh, :p2sh, :xpub, :dust, :fee_policy, :prefix, :use_timestamped_transaction)",
                use(currency.name),
                use(hex::toString(params.P2PKHVersion)),
                use(hex::toString(params.P2SHVersion)),
                use(hex::toString(params.XPUBVersion)),
                use(params.DustAmount),
                use(api::to_string(params.FeePolicy)),
                use(params.MessagePrefix),
                use(params.UsesTimestampedTransaction ? 1 : 0);
                break;
            }
            case api::WalletType::ETHEREUM:break; // TODO INSERT ETHEREUM NETWORK PARAMS
            case api::WalletType::RIPPLE:break; // TODO INSERT ETHEREUM NETWORK PARAMS
            case api::WalletType::MONERO:break; // TODO INSERT MONERO NETWORK PARAMS
        }
        inserted = true;
    }
    insertUnits(sql, currency);
    return inserted;
}

void ledger::core::CurrenciesDatabaseHelper::getAllCurrencies(soci::session &sql,
                                                              std::vector<ledger::core::api::Currency> &currencies) {
    rowset<row> rows = (sql.prepare <<
        "SELECT currencies.name, currencies.type, currencies.bip44_coin_type, currencies.payment_uri_scheme, "
        "       bitcoin_currencies.p2pkh_version, bitcoin_currencies.p2sh_version, bitcoin_currencies.xpub_version,"
        "       bitcoin_currencies.dust_amount, bitcoin_currencies.fee_policy, bitcoin_currencies.has_timestamped_transaction,"
        "       bitcoin_currencies.message_prefix "
        "FROM currencies "
        "LEFT OUTER JOIN bitcoin_currencies ON bitcoin_currencies.name = currencies.name");
    for (auto& row : rows) {
        auto offset = 0;
        api::Currency currency;
        currency.name = row.get<std::string>(0);
        currency.walletType = api::from_string<api::WalletType>(row.get<std::string>(1));
        currency.bip44CoinType = row.get<int32_t>(2);
        currency.paymentUriScheme = row.get_indicator(3) == i_null ? "" : row.get<std::string>(3);
        switch (currency.walletType) {
            case api::WalletType::BITCOIN: {
                api::BitcoinLikeNetworkParameters params;
                params.P2PKHVersion = hex::toByteArray(row.get<std::string>(4));
                params.P2SHVersion = hex::toByteArray(row.get<std::string>(5));
                params.XPUBVersion = hex::toByteArray(row.get<std::string>(6));
                params.DustAmount = row.get<int64_t>(7);
                params.FeePolicy = api::from_string<api::BitcoinLikeFeePolicy>(row.get<std::string>(8));
                params.UsesTimestampedTransaction = row.get<int>(9) == 1;
                params.MessagePrefix = row.get<std::string>(10);
                currency.bitcoinLikeNetworkParameters = params;
                break;
            }
            case api::WalletType::ETHEREUM:break;
            case api::WalletType::RIPPLE:break;
            case api::WalletType::MONERO:break;
        }
        getAllUnits(sql, currency);
        currencies.push_back(currency);
    }
}

void ledger::core::CurrenciesDatabaseHelper::getAllUnits(soci::session &sql, ledger::core::api::Currency &currency) {
    rowset<row> rows = (sql.prepare <<
        "SELECT name, magnitude, symbol, code FROM units WHERE currency_name = :currency", use(currency.name)
    );
    for (auto& row : rows) {
        api::CurrencyUnit unit;
        unit.name = row.get<std::string>(0);
        unit.numberOfDecimal = row.get<int32_t>(1);
        unit.symbol = row.get<std::string>(2);
        unit.code = row.get<std::string>(3);
        currency.units.push_back(unit);
    }
}

void
ledger::core::CurrenciesDatabaseHelper::insertUnits(soci::session &sql, const ledger::core::api::Currency &currency) {
    for (const auto& unit : currency.units) {
        int count;
        sql << "SELECT COUNT(*) FROM units WHERE name = :name", use(unit.name), into(count);
        if (count == 0) {
            sql << "INSERT INTO units VALUES(:name, :magnitude, :symbol, :code, :currency_name)",
            use(unit.name), use(unit.numberOfDecimal), use(unit.symbol), use(unit.code), use(currency.name);
        }
    }
}

void ledger::core::CurrenciesDatabaseHelper::removeCurrency(soci::session &sql, const std::string &currencyName) {
    sql << "DELETE FROM currencies WHERE name = :name", use(currencyName);
}
