/*
 *
 * keychain_test_helper
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 16/05/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include <src/wallet/bitcoin/networks.hpp>
#include <src/wallet/currencies.hpp>

#ifndef LEDGER_CORE_KEYCHAIN_TEST_HELPER_H
#define LEDGER_CORE_KEYCHAIN_TEST_HELPER_H

struct KeychainTestData {

    ledger::core::api::BitcoinLikeNetworkParameters parameters;
    ledger::core::api::Currency currency;
    std::string xpub;
    std::string derivationPath;

    KeychainTestData() = default;

    KeychainTestData(ledger::core::api::BitcoinLikeNetworkParameters parameters_,
             ledger::core::api::Currency currency_,
             std::string xpub_,
             std::string derivationPath_):
            parameters(std::move(parameters_)),
            currency(std::move(currency_)),
            xpub(std::move(xpub_)),
            derivationPath(std::move(derivationPath_))
    {}

    KeychainTestData(const KeychainTestData &data) {
        this->parameters = data.parameters;
        this->currency = data.currency;
        this->xpub = data.xpub;
        this->derivationPath = data.derivationPath;
    }

    KeychainTestData& operator=(const KeychainTestData &data) {
        this->parameters = data.parameters;
        this->currency = data.currency;
        this->xpub = data.xpub;
        this->derivationPath = data.derivationPath;
        return *this;
    }
};

extern KeychainTestData BTC_DATA;
extern KeychainTestData BTC_TESTNET_DATA;
extern KeychainTestData BCH_DATA;
extern KeychainTestData BTG_DATA;
extern KeychainTestData ZCASH_DATA;

#endif //LEDGER_CORE_KEYCHAIN_TEST_HELPER_H
