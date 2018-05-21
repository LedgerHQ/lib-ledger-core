/*
 *
 * p2sh_keychain_test
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 01/05/2018.
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

#include <gtest/gtest.h>
#include <src/wallet/bitcoin/keychains/P2SHBitcoinLikeKeychain.hpp>
//#include <src/wallet/bitcoin/networks.hpp>
//#include <src/wallet/currencies.hpp>
#include "keychain_test_helper.h"
#include "../BaseFixture.h"

#include <iostream>
using namespace std;



class BitcoinP2SHKeychains : public BaseFixture {
public:

    void testP2SHKeychain(const KeychainTestData &data, std::function<void (P2SHBitcoinLikeKeychain&)> f) {
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
            P2SHBitcoinLikeKeychain keychain(
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

TEST_F(BitcoinP2SHKeychains, KeychainDerivation) {
    testP2SHKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
    });
}

TEST_F(BitcoinP2SHKeychains, BCHKeychainDerivation) {
    testP2SHKeychain(BCH_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "386ufVmVhuv9AgakZvFno1XsKhuYyF4xGm");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "34oRCa2pifjRjMqkRhadH51BwMXedS3deg");
    });
}

TEST_F(BitcoinP2SHKeychains, BTGKeychainDerivation) {
    testP2SHKeychain(BTG_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "ASos7TVvSieocAHzjQy6K2wANaKLsi45cD");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "AHqYBQpa58aLuW6eydeKJyaq21v2oCg8TU");
    });
}

TEST_F(BitcoinP2SHKeychains, ZCASHKeychainDerivation) {
    testP2SHKeychain(ZCASH_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "t3fyoi4hioJYbsUtt2rPamWTpzTZLQUddjz");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "t3VrHbQGP1jTVpqbWGcaXY4Z5BHMDYa19xt");
    });
}

//TEST_F(BitcoinP2SHKeychains, ZENCASHKeychainDerivation) {
//    testP2SHKeychain(ZENCASH_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
//        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "t3TYay3EKTp8Mn47TUwqvv66M6Q7AEaUMRi");
//        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "t3NTdnrqvcfA6yBibbULYdoVMhSttcF4rHX");
//    });
//}

TEST_F(BitcoinP2SHKeychains, SimpleUsedReceiveAddresses) {
    testP2SHKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_FALSE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ");
    });
}

TEST_F(BitcoinP2SHKeychains, SimpleUsedChangeAddresses) {
    testP2SHKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_FALSE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji");
    });
}

TEST_F(BitcoinP2SHKeychains, NonConsecutivesReceiveUsed) {
    testP2SHKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2N8BfQZXJPJetK6GuBhfWaKmgQxTzKuAa4j");
    });
}

TEST_F(BitcoinP2SHKeychains, NonConsecutivesChangeUsed) {
    testP2SHKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2NDQh3V5suxHfzXbFXyroDYjLVRZB8WCAPc");
    });
}

TEST_F(BitcoinP2SHKeychains, CheckIfEmpty) {
    testP2SHKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
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