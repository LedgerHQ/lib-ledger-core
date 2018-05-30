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

#include "keychain_test_helper.h"
#include "../BaseFixture.h"



class BitcoinKeychains : public BaseFixture {
public:
    void testP2PKHKeychain(const KeychainTestData &data, std::function<void (P2PKHBitcoinLikeKeychain&)> f) {
        auto backend = std::make_shared<ledger::core::PreferencesBackend>(
            "/preferences/tests.db",
            dispatcher->getMainExecutionContext(),
            resolver
        );
        auto configuration = std::make_shared<DynamicObject>();
        dispatcher->getMainExecutionContext()->execute(ledger::qt::make_runnable([=]() {
            P2PKHBitcoinLikeKeychain keychain(
                    configuration,
                    data.currency,
                    0,
                    ledger::core::BitcoinLikeExtendedPublicKey::fromBase58(data.currency, data.xpub, optional<std::string>(data.derivationPath)),
                    backend->getPreferences("keychain")
            );
            f(keychain);
            dispatcher->stop();
        }));
        dispatcher->waitUntilStopped();
    };
};

TEST_F(BitcoinKeychains, KeychainDerivation) {
    testP2PKHKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
    });
}

TEST_F(BitcoinKeychains, BCHKeychainDerivation) {
    testP2PKHKeychain(BCH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "1BW6hLyZKY9AnUwrU9CwHQJ2c79ho49q4f");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1ETcGdzh7XGgP2HRUqkYuYEMifihTt8ZiF");
    });
}

TEST_F(BitcoinKeychains, BTGKeychainDerivation) {
    testP2PKHKeychain(BTG_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "GeB2eVacdg6T5U4beqXZgL6vPPPspgqBic");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "GWzECqesKKgjH5RXVt9Na5MYbH2BmeKveF");
    });
}

TEST_F(BitcoinKeychains, ZCASHKeychainDerivation) {
    testP2PKHKeychain(ZCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "t1Y1C1GiyhffDV3AMCAcBdYT2H2J9ng2eoY");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "t1eHTEWDziaKtBqaZrSqAPgvm6tBGZezgBR");
    });
}

TEST_F(BitcoinKeychains, ZENCASHKeychainDerivation) {
    testP2PKHKeychain(ZENCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "znSHF3jfG9REjJWjFoRrr1WWpNFj4NWeyCW");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "zno29sgBi7CdERRqqjs39mHTdkRHhuodZDa");
    });
}

TEST_F(BitcoinKeychains, LTCKeychainDerivation) {
    testP2PKHKeychain(LTC_DATA_LEGACY, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "LSPT2mGEFnHg9wUZ1dqzFbjxfAtgYizGgT");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "LhUaNteNLDsuM3BiTGJusHkNsLWkLKpgJ5");
    });
}

TEST_F(BitcoinKeychains, PEERCOINKeychainDerivation) {
    testP2PKHKeychain(PEERCOIN_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "P9FBRWVkjHwjituqUzqxDrRSuhv7uoaP6o");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "PFBBNu1g73ppb8pf7fFJjrhmAWkHdnkxda");
    });
}

TEST_F(BitcoinKeychains, SimpleUsedReceiveAddresses) {
    testP2PKHKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_FALSE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb");
    });
}

TEST_F(BitcoinKeychains, SimpleUsedChangeAddresses) {
    testP2PKHKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_FALSE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD");
    });
}

TEST_F(BitcoinKeychains, NonConsecutivesReceiveUsed) {
    testP2PKHKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "1GJr9FHZ1pbR4hjhX24M4L1BDUd2QogYYA");
    });
}

TEST_F(BitcoinKeychains, NonConsecutivesChangeUsed) {
    testP2PKHKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1F2arsfX5JEDryBVftmzbVFWaGsJaTVwcg");
    });
}

TEST_F(BitcoinKeychains, CheckIfEmpty) {
    testP2PKHKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_TRUE(keychain.isEmpty());
        auto addresses = keychain.getAllObservableAddresses(0, 40);
        EXPECT_TRUE(keychain.isEmpty());
        keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE);
        EXPECT_TRUE(keychain.isEmpty());
        keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE);
        EXPECT_TRUE(keychain.isEmpty());
        EXPECT_TRUE(keychain.markAsUsed(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58()));
        EXPECT_FALSE(keychain.isEmpty());
    });
}