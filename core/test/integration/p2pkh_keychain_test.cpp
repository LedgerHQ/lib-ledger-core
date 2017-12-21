/*
 *
 * p2pkh_keychain_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/01/2017.
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

#include <gtest/gtest.h>
#include <src/wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>
#include <src/wallet/bitcoin/networks.hpp>
#include <src/wallet/currencies.hpp>
#include "BaseFixture.h"

const std::string XPUB = "xpub6DCi5iJ57ZPd5qPzvTm5hUt6X23TJdh9H4NjNsNbt7t7UuTMJfawQWsdWRFhfLwkiMkB1rQ4ZJWLB9YBnzR7kbs9N8b2PsKZgKUHQm1X4or";

class BitcoinKeychains : public BaseFixture {
public:
    void testP2PKHKeychain(std::string xpub, std::function<void (P2PKHBitcoinLikeKeychain&)> f) {
        auto backend = std::make_shared<ledger::core::PreferencesBackend>(
            "/preferences/tests.db",
            dispatcher->getMainExecutionContext(),
            resolver
        );
        auto configuration = std::make_shared<DynamicObject>();
        dispatcher->getSerialExecutionContext("worker")->execute(ledger::qt::make_runnable([=]() {
            P2PKHBitcoinLikeKeychain keychain(
                    configuration,
                    ledger::core::currencies::BITCOIN,
                    0,
                    api::BitcoinLikeExtendedPublicKey::fromBase58(networks::BITCOIN, xpub, optional<std::string>("44'/0'/0'")),
                    backend->getPreferences("keychain")
            );
            f(keychain);
            dispatcher->getSerialExecutionContext("worker")->execute(ledger::qt::make_runnable([=] () {
                dispatcher->stop();
            }));
        }));
        dispatcher->waitUntilStopped();
    };
};

TEST_F(BitcoinKeychains, KeychainDerivation) {
    testP2PKHKeychain(XPUB, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
    });
}

TEST_F(BitcoinKeychains, SimpleUsedReceiveAddresses) {
    testP2PKHKeychain(XPUB, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_FALSE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb");
    });
}

TEST_F(BitcoinKeychains, SimpleUsedChangeAddresses) {
    testP2PKHKeychain(XPUB, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_FALSE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD");
    });
}

TEST_F(BitcoinKeychains, NonConsecutivesReceiveUsed) {
    testP2PKHKeychain(XPUB, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "1GJr9FHZ1pbR4hjhX24M4L1BDUd2QogYYA");
    });
}

TEST_F(BitcoinKeychains, NonConsecutivesChangeUsed) {
    testP2PKHKeychain(XPUB, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "1F2arsfX5JEDryBVftmzbVFWaGsJaTVwcg");
    });
}

TEST_F(BitcoinKeychains, CheckIfEmpty) {
    testP2PKHKeychain(XPUB, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_TRUE(keychain.isEmpty());
        auto addresses = keychain.getAllObservableAddresses(0, 40);
        EXPECT_TRUE(keychain.isEmpty());
        keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE);
        EXPECT_TRUE(keychain.isEmpty());
        keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE);
        EXPECT_TRUE(keychain.isEmpty());
        EXPECT_TRUE(keychain.markAsUsed(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)));
        EXPECT_FALSE(keychain.isEmpty());
    });
}