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
#include <src/wallet/bitcoin/networks.hpp>
#include <src/wallet/currencies.hpp>
#include "../BaseFixture.h"

#include <iostream>



using namespace std;
//abandon
//bip44
//const std::string XPUB = "tpubDC5FSnBiZDMmhiuCmWAYsLwgLYrrT9rAqvTySfuCCrgsWz8wxMXUS9Tb9iVMvcRbvFcAHGkMD5Kx8koh4GquNGNTfohfk7pgjhaPCdXpoba";
//bip49
//const std::string XPUB = "upub5EFU65HtV5TeiSHmZZm7FUffBGy8UKeqp7vw43jYbvZPpoVsgU93oac7Wk3u6moKegAEWtGNF8DehrnHtv21XXEMYRUocHqguyjknFHYfgY";

const std::string XPUB = "tpubDCcvqEHx7prGddpWTfEviiew5YLMrrKy4oJbt14teJZenSi6AYMAs2SNXwYXFzkrNYwECSmobwxESxMCrpfqw4gsUt88bcr8iMrJmbb8P2q";

class BitcoinP2SHKeychains : public BaseFixture {
public:
    void testP2SHKeychain(std::string xpub, std::function<void (P2SHBitcoinLikeKeychain&)> f) {
        auto backend = std::make_shared<ledger::core::PreferencesBackend>(
                "/preferences/tests.db",
                dispatcher->getMainExecutionContext(),
                resolver
        );
        auto xPubBtc = api::BitcoinLikeExtendedPublicKey::fromBase58(networks::BITCOIN_TESTNET, xpub, optional<std::string>("49'/1'/6'"));

        auto configuration = std::make_shared<DynamicObject>();
        dispatcher->getMainExecutionContext()->execute(ledger::qt::make_runnable([=]() {
            P2SHBitcoinLikeKeychain keychain(
                    configuration,
                    ledger::core::currencies::BITCOIN_TESTNET,
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
    testP2SHKeychain(XPUB, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
    });
}

TEST_F(BitcoinP2SHKeychains, SimpleUsedReceiveAddresses) {
    testP2SHKeychain(XPUB, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_FALSE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ");
    });
}

TEST_F(BitcoinP2SHKeychains, SimpleUsedChangeAddresses) {
    testP2SHKeychain(XPUB, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_FALSE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji");
    });
}

TEST_F(BitcoinP2SHKeychains, NonConsecutivesReceiveUsed) {
    testP2SHKeychain(XPUB, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "2N8BfQZXJPJetK6GuBhfWaKmgQxTzKuAa4j");
    });
}

TEST_F(BitcoinP2SHKeychains, NonConsecutivesChangeUsed) {
    testP2SHKeychain(XPUB, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE), "2NDQh3V5suxHfzXbFXyroDYjLVRZB8WCAPc");
    });
}

TEST_F(BitcoinP2SHKeychains, CheckIfEmpty) {
    testP2SHKeychain(XPUB, [] (P2SHBitcoinLikeKeychain& keychain) {
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