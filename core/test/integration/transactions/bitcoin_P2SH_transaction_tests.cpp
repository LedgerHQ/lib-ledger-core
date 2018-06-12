/*
 *
 * bitcoin_P2SH_transaction_tests.cpp
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 02/05/2018.
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

#include "../BaseFixture.h"
#include "../../fixtures/testnet_xpub_fixtures.h"
#include "../../fixtures/btg_xpub_fixtures.h"

//#include <src/wallet/pool/WalletPool.hpp>
#include <api/KeychainEngines.hpp>
#include "transaction_test_helper.h"


#include <iostream>
using namespace std;

#include <utils/hex.h>

struct BitcoinMakeP2SHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_testnet";
        testData.inflate = ledger::testing::testnet_xpub::inflate;
    }
};

TEST_F(BitcoinMakeP2SHTransaction, CreateStandardP2SHWithOneOutput) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 200000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 71));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize());
    //auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

TEST_F(BitcoinMakeP2SHTransaction, CreateStandardP2SHWithWipeToAddress) {
    auto builder = tx_builder();
    builder->wipeToAddress("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 71));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto outputs = tx->getOutputs();
    auto fees = tx->getFees();
    auto balance = wait(account->getBalance());
    EXPECT_EQ(outputs.size(), 1);
    auto maxAmount = outputs[0]->getValue();
    EXPECT_EQ(balance->toLong(), maxAmount->toLong() + fees->toLong());
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

struct BTGMakeP2SHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_gold";
        testData.inflate = ledger::testing::btg_xpub::inflate;
    }
};

TEST_F(BTGMakeP2SHTransaction, CreateStandardP2SHWithOneOutput) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 2000), "ATqSa4V9ZjxPxDBe877bbXeKMfZA644mBk");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 41));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize());
    //auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}
