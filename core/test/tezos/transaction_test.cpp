/*
 *
 * tezos/transaction_tests
 *
 * Created by Axel Haustant on 10/09/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "transaction_test.hpp"
#include "Fixtures.hpp"
// #include "../BaseFixture.h"
// #include "../../fixtures/xtz_fixtures.h"
#include <api/DynamicObject.hpp>
#include <api/KeychainEngines.hpp>
#include <api/TezosLikeOriginatedAccount.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <api/BlockchainExplorerEngines.hpp>
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/currencies.hpp>
#include <iostream>
#include <wallet/tezos/tezosNetworks.h>

using namespace std;
using namespace ledger::testing::tezos;

void TezosMakeBaseTransaction::SetUp() {
    BaseFixture::SetUp();
    recreate();
}

void TezosMakeBaseTransaction::recreate() {
    SetUpConfig();
    pool = newDefaultPool();
    wallet = wait(pool->createWallet(testData.walletName, testData.currencyName, testData.configuration));
    account = testData.inflate_xtz(pool, wallet);
    currency = wallet->getCurrency();
}

void TezosMakeBaseTransaction::TearDown() {
    BaseFixture::TearDown();
    pool = nullptr;
    wallet = nullptr;
    account = nullptr;
}

void TezosMakeBaseTransaction::broadcast(std::shared_ptr<TezosLikeTransactionApi> tx) {
    dispatcher = std::make_shared<QtThreadDispatcher>();
    auto callback = std::make_shared<Callback>(dispatcher);  
    account->broadcastTransaction(tx, callback);
    dispatcher->waitUntilStopped();
}

void TezosMakeBaseTransaction::broadcast(const std::vector<uint8_t> &raw) {
    dispatcher = std::make_shared<QtThreadDispatcher>();
    auto callback = std::make_shared<Callback>(dispatcher);  
    account->broadcastRawTransaction(raw, callback);
    dispatcher->waitUntilStopped();
}

std::shared_ptr<TezosLikeTransactionBuilder> TezosMakeBaseTransaction::tx_builder() {
    return std::dynamic_pointer_cast<TezosLikeTransactionBuilder>(account->buildTransaction());
}


struct TransactionTest : public TezosBaseTest {};

/*
TEST_F(TransactionTest, ParseUnsignedRawRevealTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94070000d2e495a7ab40156d0a7c35b73d2530a3470fc870ea0902904e0000cda3081bd81219ec494b29068dcfd19e427fed9a66abcdc9e9e99ca6478f60e9";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1258L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10000L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(TransactionTest, ParseUnsignedRawOriginationTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94090000d2e495a7ab40156d0a7c35b73d2530a3470fc870920903f44e950200d2e495a7ab40156d0a7c35b73d2530a3470fc8708094ebdc03ffff0000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1170L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10100L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "277");
}

TEST_F(TransactionTest, ParseUnsignedRawDelegationTransaction) {
    // round-trip
    auto strTx = "037d8d230a91d1fb8391727f37a1bfeb332b7f249c78315ea4ae934e2103a826630a01d315f72434520d43d415f0dff4632519501d2d9400890902f44e00ff008bd703c4a2d91b8f1d79455be9b99c2693e931fd";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "KT1TptTRYx2BEYetm61ABjkBdfHXQ2SQrXo8");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1YPSCGWXwBdTncK2aCctSZAXWvGsGwVJqU");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1161L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10100L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

// Reference https://tzscan.io/ooPbtVVy7TZLoRirGsCgyy6Esyqm3Kj22QvEVpAmEXX3vHBGbF8
TEST_F(TransactionTest, ParseSignedRawDelegationTransaction) {
    // round-trip
    auto strTx = "4fd4dca725498e819cc4dd6a87adcfb98770f600558d5d097d1a48b2324a9a2e080000bbdd4268871d1751a601fe66603324714266bf558c0bdeff4ebc50ac02a08d0601583f106387cb85212812b738cae45b497551bf9a00007dc21f46b94d6b432c881b78e1fee917ef0fd382571a5d5f1bf1c5aa90d62a02777eeebac53e3fe9bbcf4501cc2ee0cb1dbe65ec24c869a4715d84f65cfdc101";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawSignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1cmN7N6rV9ULVqbL2BxSUZgeL5wnWyoBUE");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "KT1GdNaQowD3r8VprK8pni2R2DZd5Vxnkvw5");
    // It is BLKSXLjQkkPj3WsRZQ6PAwDotSbR25t3mKvZ7bvXUx75Poex4u9 predecessor of
    // BM7DAEUj8dCNUzViXjg8DyeWtthvorDXMk6ocMwNz7iVo73WNKC where tx included
    EXPECT_EQ(tx->getBlockHash().value(), "BLKSXLjQkkPj3WsRZQ6PAwDotSbR25t3mKvZ7bvXUx75Poex4u9");
    EXPECT_EQ(tx->getValue()->toLong(), 100000L);
    EXPECT_EQ(tx->getFees()->toLong(), 1420L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10300L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "300");
    EXPECT_EQ(tx->getCounter()->toString(10), "1294302");
    

}
*/