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

#include "api/KeychainEngines.hpp"
#include "api/PreferencesChange.hpp"
#include "keychain_test_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class MockPreferencesBackend : public api::PreferencesBackend {
  public:
    MOCK_METHOD(std::experimental::optional<std::vector<uint8_t>>, get, (const std::vector<uint8_t> &));
    MOCK_METHOD(bool, commit, (const std::vector<api::PreferencesChange> &));
    MOCK_METHOD(void, setEncryption, (const std::shared_ptr<api::RandomNumberGenerator> &, const std::string &));
    MOCK_METHOD(void, unsetEncryption, ());
    MOCK_METHOD(bool, resetEncryption, (const std::shared_ptr<api::RandomNumberGenerator> &, const std::string &, const std::string &));
    MOCK_METHOD(std::string, getEncryptionSalt, ());
    MOCK_METHOD(void, clear, ());
};

class ConcreteCommonBitcoinLikeKeychains : public CommonBitcoinLikeKeychains {
  public:
    ConcreteCommonBitcoinLikeKeychains(
        const std::shared_ptr<api::DynamicObject> &configuration,
        const api::Currency &params,
        int account,
        const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
        const std::shared_ptr<Preferences> &preferences)
        : CommonBitcoinLikeKeychains(configuration, params, account, xpub, preferences) {
        _keychainEngine = api::KeychainEngines::BIP49_P2SH;
    }
    int32_t getOutputSizeAsSignedTxInput() const override { return 0; }
};

class CommonBitcoinKeychains : public KeychainFixture<ConcreteCommonBitcoinLikeKeychains> {
};

template <typename T>
std::vector<T> string2vector(const std::string &s) {
    return std::vector<T>(s.begin(), s.end());
}

std::string serializeStateToString(const KeychainPersistentState &state) {
    std::stringstream is;
    ::cereal::BinaryOutputArchive archive(is);
    archive(state);
    return is.str();
}

std::vector<uint8_t> serializeStateToVector(const KeychainPersistentState &state) {
    const std::string savedState = serializeStateToString(state);
    return std::vector<uint8_t>((const uint8_t *)savedState.data(), (const uint8_t *)savedState.data() + savedState.size());
}

TEST_F(CommonBitcoinKeychains, DISABLED_CorrectStateAtInitialization) {

    KeychainPersistentState mockState;
    mockState.maxConsecutiveChangeIndex = 4;
    mockState.nonConsecutiveChangeIndexes.emplace(1);
    mockState.nonConsecutiveChangeIndexes.emplace(2);
    mockState.nonConsecutiveChangeIndexes.emplace(3);
    mockState.maxConsecutiveReceiveIndex = 4;
    mockState.nonConsecutiveReceiveIndexes.emplace(1);
    mockState.nonConsecutiveReceiveIndexes.emplace(2);
    mockState.nonConsecutiveReceiveIndexes.emplace(3);

    auto backend = std::make_shared<MockPreferencesBackend>();
    EXPECT_CALL(*backend, get(string2vector<uint8_t>("keychainstate")))
        .Times(1)
        .WillOnce(Return(serializeStateToVector(mockState)));

    testKeychain(BTC_TESTNET_DATA, backend, [&backend, &mockState](CommonBitcoinLikeKeychains &keychain) {
        const KeychainPersistentState &state = keychain.getState();
        EXPECT_EQ(mockState.maxConsecutiveChangeIndex, state.maxConsecutiveChangeIndex);
        EXPECT_EQ(mockState.maxConsecutiveReceiveIndex, state.maxConsecutiveReceiveIndex);
        EXPECT_EQ(mockState.nonConsecutiveChangeIndexes.size(), state.nonConsecutiveChangeIndexes.size());
        EXPECT_EQ(mockState.nonConsecutiveReceiveIndexes.size(), state.nonConsecutiveReceiveIndexes.size());
    });
}

TEST_F(CommonBitcoinKeychains, CorrectStateProducedByMarkPathAsUsed) {

    testKeychain(BTC_TESTNET_DATA, [](CommonBitcoinLikeKeychains &keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji"));

        const KeychainPersistentState &state = keychain.getState();
        EXPECT_EQ(0, state.maxConsecutiveChangeIndex);
        EXPECT_EQ(0, state.maxConsecutiveReceiveIndex);
        EXPECT_EQ(1, state.nonConsecutiveChangeIndexes.size());
        EXPECT_EQ(0, state.nonConsecutiveReceiveIndexes.size());
    });
}

TEST_F(CommonBitcoinKeychains, CorrectStateUsedAtMarkPathAsUsed) {

    testKeychain(BTC_TESTNET_DATA, [](ConcreteCommonBitcoinLikeKeychains &keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);

        // update state from outside
        KeychainPersistentState stateInput = keychain.getState();
        stateInput.maxConsecutiveChangeIndex = 0;
        stateInput.nonConsecutiveChangeIndexes.insert(0);
        keychain.getPreferences()->edit()->putData("state", serializeStateToVector(stateInput))->commit();

        EXPECT_TRUE(keychain.markAsUsed("2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji"));

        const KeychainPersistentState &state = keychain.getState();
        EXPECT_EQ(2, state.maxConsecutiveChangeIndex);
        EXPECT_EQ(0, state.maxConsecutiveReceiveIndex);
        EXPECT_EQ(0, state.nonConsecutiveChangeIndexes.size());
        EXPECT_EQ(0, state.nonConsecutiveReceiveIndexes.size());
    });
}
