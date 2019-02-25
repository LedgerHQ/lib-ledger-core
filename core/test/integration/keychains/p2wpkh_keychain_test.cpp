/*
 *
 * p2wpkh_keychain_test
 *
 * Created by El Khalil Bellakrid on 24/02/2019.
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
#include <gtest/gtest.h>
#include <iostream>
#include <src/wallet/bitcoin/keychains/P2WPKHBitcoinLikeKeychain.hpp>
#include "keychain_test_helper.h"
#include "../BaseFixture.h"

using namespace std;

class BitcoinP2WPKHKeychains : public BaseFixture {
public:
    void testP2WPKHKeychain(const KeychainTestData &data, std::function<void (P2WPKHBitcoinLikeKeychain&)> f) {
        auto backend = std::make_shared<ledger::core::PreferencesBackend>(
                "/preferences/tests.db",
                dispatcher->getMainExecutionContext(),
                resolver
        );
        auto xPubBtc = ledger::core::BitcoinLikeExtendedPublicKey::fromBase58(data.currency,
                                                                              data.xpub,
                                                                              optional<std::string>(data.derivationPath));

        auto configuration = std::make_shared<DynamicObject>();
        dispatcher->getMainExecutionContext()->execute(ledger::qt::make_runnable([=]() {
            P2WPKHBitcoinLikeKeychain keychain(
                    configuration,
                    data.currency,
                    0,
                    xPubBtc,
                    backend->getPreferences("keychain")
            );
            f(keychain);
            dispatcher->stop();
        }));
        dispatcher->waitUntilStopped();
    };
};

TEST_F(BitcoinP2WPKHKeychains, KeychainDerivation) {
    testP2WPKHKeychain(BTC_TESTNET_DATA, [] (P2WPKHBitcoinLikeKeychain& keychain) {
        auto freshAddress = keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE);
        EXPECT_EQ(freshAddress->toBech32(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
    });
}