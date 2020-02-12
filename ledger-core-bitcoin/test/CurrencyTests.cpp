/*
 *
 * CurrencyTests
 * ledger-core-bitcoin
 *
 * Created by Alexis Le Provost on 04/02/2020.
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

#include <unordered_set>

#include <gtest/gtest.h>

#include <core/Services.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/wallet/CurrencyBuilder.hpp>

#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/BitcoinNetworks.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>

#include <integration/WalletFixture.hpp>

class BitcoinCurrencies : public WalletFixture<BitcoinLikeWalletFactory> {

};

TEST_F(BitcoinCurrencies, InitializeCurrencies) {
    api::Currency targetedCurrency;

    // registers several currencies in purpose
    registerCurrency(currencies::litecoin());
    registerCurrency(currencies::bitcoin());
    registerCurrency(currencies::qtum());

    for (auto& currency : walletStore->getCurrencies()) {
        if (currency.name == "bitcoin") {
            targetedCurrency = currency;
        }
    }

    auto parameters = networks::getBitcoinLikeNetworkParameters(targetedCurrency.name);

    EXPECT_EQ(targetedCurrency.name, "bitcoin");
    EXPECT_EQ(targetedCurrency.bip44CoinType, 0);
    EXPECT_EQ(targetedCurrency.paymentUriScheme, "bitcoin");
    EXPECT_EQ(parameters.P2PKHVersion[0], 0);
    EXPECT_EQ(parameters.P2SHVersion[0], 5);

    for (const auto& unit : targetedCurrency.units) {
        if (unit.name == "bitcoin") {
            EXPECT_EQ(unit.code, "BTC");
            EXPECT_EQ(unit.numberOfDecimal, 8);
        } else if (unit.name == "milli-bitcoin") {
            EXPECT_EQ(unit.code, "mBTC");
            EXPECT_EQ(unit.numberOfDecimal, 5);
        }
    }
}

TEST_F(BitcoinCurrencies, AddCurrency) {
    api::Currency newCurrency = CurrencyBuilder("wonder_coin")
        .bip44(42)
        .unit("wonder coin", 12, "WC");

    registerCurrency(newCurrency);

    bool found = false;

    for (const auto &currency : walletStore->getCurrencies()) {
        if (currency.name == newCurrency.name) {
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(BitcoinCurrencies, RemoveCurrency) {
    api::Currency newCurrency = CurrencyBuilder("wonder_coin")
        .bip44(42)
        .unit("wonder coin", 12, "WC");

    registerCurrency(newCurrency);

    {
        bool found = false;

        for (const auto &currency : walletStore->getCurrencies()) {
            if (currency.name == newCurrency.name) {
                found = true;
            }
        }

        EXPECT_TRUE(found);
    }

    {
        bool found = false;

        wait(walletStore->removeCurrency(newCurrency.name));
     
        for (const auto &currency : walletStore->getCurrencies()) {
            if (currency.name == newCurrency.name) {
                found = true;
            }
        }
        EXPECT_FALSE(found);
    }
}