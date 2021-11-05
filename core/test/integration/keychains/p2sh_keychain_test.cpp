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
#include <gmock/gmock.h>
#include <iostream>
#include <src/wallet/bitcoin/keychains/P2SHBitcoinLikeKeychain.hpp>
#include <src/api/PreferencesChange.hpp>
#include "keychain_test_helper.h"

using namespace std;
using ::testing::_;
using ::testing::Return;

class BitcoinP2SHKeychains : public KeychainFixture<P2SHBitcoinLikeKeychain> {

};

TEST_F(BitcoinP2SHKeychains, KeychainDerivation) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
    });
}

TEST_F(BitcoinP2SHKeychains, BCHKeychainDerivation) {
    testKeychain(BCH_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "386ufVmVhuv9AgakZvFno1XsKhuYyF4xGm");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "34oRCa2pifjRjMqkRhadH51BwMXedS3deg");
    });
}

TEST_F(BitcoinP2SHKeychains, BTGKeychainDerivation) {
    testKeychain(BTG_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "ASos7TVvSieocAHzjQy6K2wANaKLsi45cD");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "AHqYBQpa58aLuW6eydeKJyaq21v2oCg8TU");
    });
}

TEST_F(BitcoinP2SHKeychains, ZCASHKeychainDerivation) {
    testKeychain(ZCASH_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "t3fyoi4hioJYbsUtt2rPamWTpzTZLQUddjz");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "t3VrHbQGP1jTVpqbWGcaXY4Z5BHMDYa19xt");
    });
}

TEST_F(BitcoinP2SHKeychains, LTCKeychainDerivation) {
    testKeychain(LTC_DATA_SEGWIT, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "MJGpPKDPeiArQt1UMy8NhcEccdJuRNkbMj");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "MFJ6m85UMdEyRwEem2bziaxTECyKKWnjGp");
    });
}

TEST_F(BitcoinP2SHKeychains, VertCoinKeychainDerivation) {
    testKeychain(VTC_DATA_SEGWIT, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "32Q13F9yNnp55Li3oJ832EnTY5D2skVV6M");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "31jepsZDaBeRzdf8qcrV83gZY4GHKuamPe");
    });
}

TEST_F(BitcoinP2SHKeychains, ViaCoinKeychainDerivation) {
    testKeychain(VIA_DATA_SEGWIT, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "EMJaVjLFKoB4a5F256DxiMNh6SCR7Um1dZ");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "Ec6sBX4MZDBitLJ9sk6GZ6sfNzEzfReuzA");
    });
}

TEST_F(BitcoinP2SHKeychains, SimpleUsedReceiveAddresses) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_FALSE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ");
    });
}

TEST_F(BitcoinP2SHKeychains, SimpleUsedChangeAddresses) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_FALSE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji");
    });
}

TEST_F(BitcoinP2SHKeychains, NonConsecutivesReceiveUsed) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2N8BfQZXJPJetK6GuBhfWaKmgQxTzKuAa4j");
    });
}

TEST_F(BitcoinP2SHKeychains, NonConsecutivesChangeUsed) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2NDQh3V5suxHfzXbFXyroDYjLVRZB8WCAPc");
    });
}

TEST_F(BitcoinP2SHKeychains, CheckIfEmpty) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
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


class MockPreferencesBackend: public api::PreferencesBackend {
 public:
    //MockPreferencesBackend(const string&, const std::shared_ptr<ledger::core::api::ExecutionContext>&, const std::shared_ptr<ledger::core::api::PathResolver>&) {}
    MOCK_METHOD(std::experimental::optional<std::vector<uint8_t>>, get, (const std::vector<uint8_t> &));
    MOCK_METHOD(bool, commit, (const std::vector<api::PreferencesChange> &));
    MOCK_METHOD(void, setEncryption, (const std::shared_ptr<api::RandomNumberGenerator> &, const std::string &));
    MOCK_METHOD(void, unsetEncryption, ());
    MOCK_METHOD(bool, resetEncryption, (const std::shared_ptr<api::RandomNumberGenerator> &, const std::string &, const std::string &));
    MOCK_METHOD(std::string, getEncryptionSalt, ());
    MOCK_METHOD(void, clear, ());

};


class ConcreteCommonBitcoinLikeKeychains: public CommonBitcoinLikeKeychains {
public:
    ConcreteCommonBitcoinLikeKeychains(const std::shared_ptr<api::DynamicObject> &configuration,
                                                           const api::Currency &params,
                                                           int account,
                                                           const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                                                           const std::shared_ptr<Preferences> &preferences) 
                                                           : CommonBitcoinLikeKeychains(configuration, params, account, xpub, preferences)
                                                           {}
    int32_t getOutputSizeAsSignedTxInput() const override { return 0; }
};

class CommonBitcoinKeychains : public KeychainFixture<ConcreteCommonBitcoinLikeKeychains> {
};

template <typename T>
std::vector<T> string2vector(const std::string& s) {
    return std::vector<T>(s.begin(), s.end());
} 

std::vector<uint8_t> serializeState(const KeychainPersistentState& state) {
    std::stringstream is;
    ::cereal::BinaryOutputArchive archive(is);
    archive(state);
    auto savedState = is.str();
    return std::vector<uint8_t>((const uint8_t *)savedState.data(),(const uint8_t *)savedState.data() + savedState.size());
}

TEST_F(CommonBitcoinKeychains, UpdatesInternalStateAtInitialization) {

    auto backend = std::make_shared<MockPreferencesBackend>(); 

    KeychainPersistentState mockState;
    mockState.maxConsecutiveChangeIndex = 4;
    mockState.nonConsecutiveChangeIndexes.emplace(1);
    mockState.nonConsecutiveChangeIndexes.emplace(2);
    mockState.nonConsecutiveChangeIndexes.emplace(3);
    mockState.maxConsecutiveReceiveIndex = 4;
    mockState.nonConsecutiveReceiveIndexes.emplace(1);
    mockState.nonConsecutiveReceiveIndexes.emplace(2);
    mockState.nonConsecutiveReceiveIndexes.emplace(3);

    EXPECT_CALL(*backend, get(string2vector<uint8_t>("keychainstate")))
        .Times(1)
        .WillOnce(Return(serializeState(mockState)));

    testKeychain(BTC_TESTNET_DATA, backend, [&backend, &mockState] (CommonBitcoinLikeKeychains& keychain) {
        const KeychainPersistentState& state = keychain.getState();
        EXPECT_EQ(mockState.maxConsecutiveChangeIndex, state.maxConsecutiveChangeIndex);
        EXPECT_EQ(mockState.maxConsecutiveReceiveIndex, state.maxConsecutiveReceiveIndex);
        EXPECT_EQ(mockState.nonConsecutiveChangeIndexes.size(), state.nonConsecutiveChangeIndexes.size());
        EXPECT_EQ(mockState.nonConsecutiveReceiveIndexes.size(), state.nonConsecutiveReceiveIndexes.size());
    });
}
