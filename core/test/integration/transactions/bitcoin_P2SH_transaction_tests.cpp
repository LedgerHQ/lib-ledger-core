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
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <api/KeychainEngines.hpp>
#include "transaction_test_helper.h"

#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <crypto/HASH160.hpp>

#include <iostream>
using namespace std;
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
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
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
    auto txSerialized = tx->serialize();
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), txSerialized, 0);
    auto parsedTxSerialized = parsedTx->serialize();
    cout<<"tx->serialize(): "<<hex::toString(txSerialized)<<endl;
    cout<<"parsedTx->serialize(): "<<hex::toString(parsedTxSerialized)<<endl;
    EXPECT_EQ(txSerialized, parsedTxSerialized);
}

TEST_F(BitcoinMakeP2SHTransaction, ParseSignedRawTransaction) {
    //Tx hash 93ae1990d10745e3ab4bf742d4b06bd513e7a26384617a17525851e4e3ed7038
    auto strTx = "0100000000010182815d16259062c4bc08c4fb3aa985444b7208197cf676212f1b3da93782e19f0100000017160014e4fae08faaa8469c5756fda7fbfde46922a4e7b2ffffffff0280f0fa020000000017a91428242bc4e7266060e084fab55fb70916b605d0b3870f4a9b040000000017a91401445204b7063c76c702501899334d6f7499806d870248304502210085a85a2dec818ece4748c0c9d71640a5703a5eec9112dd58183b048c6a9961cf02201dd0beabc1c3500f75849a046deab9e2d2ce388fff6cb92693508f5ea406471d012103d2f424cd1f60e96241a968b9da3c3f6b780f90538bdf306350b9607c279ad48600000000";
    auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(currency, hex::toByteArray(strTx), 0);
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
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
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    //auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}
