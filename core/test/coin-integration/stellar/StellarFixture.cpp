/*
 *
 * StellarFixture.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/02/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include "StellarFixture.hpp"

static std::vector<api::CurrencyUnit> UNITS {};

static api::StellarLikeNetworkParameters STELLAR_PARAMS {
    "xlm", {6 << 3}, 5000000, 100, {}
};

static api::Currency STELLAR =
        Currency("stellar")
        .forkOfStellar(STELLAR_PARAMS)
        .bip44(148)
        .paymentUri("web+stellar")
        .unit("stroops", 0, "stroops")
        .unit("lumen", 7, "XLM");

api::AccountCreationInfo StellarFixture::defaultAccount() const {
    return api::AccountCreationInfo(0, {"main"}, {"44'/0'/0/0"}, {}, {});
}

std::shared_ptr<WalletPool> StellarFixture::newPool(std::string poolName) {
    auto pool = CoinIntegrationFixture::newPool(poolName);
    injectCurrency(pool, STELLAR);
    return pool;
}

api::Currency StellarFixture::getCurrency() const {
    return STELLAR;
}

