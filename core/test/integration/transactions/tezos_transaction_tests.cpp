/*
 *
 * tezos_transaction_tests
 *
 * Created by El Khalil Bellakrid on 02/05/2019.
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

#include "../BaseFixture.h"
#include "../../fixtures/xtz_fixtures.h"
#include <api/KeychainEngines.hpp>
#include <api/TezosLikeOriginatedAccount.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <api/BlockchainExplorerEngines.hpp>
#include "transaction_test_helper.h"
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <wallet/currencies.hpp>
#include <iostream>
using namespace std;

struct TezosMakeTransaction : public TezosMakeBaseTransaction {
    void SetUpConfig() override {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
        testData.configuration = configuration;
        testData.walletName = randomWalletName();
        testData.currencyName = "tezos";
        testData.inflate_xtz = ledger::testing::xtz::inflate;
    }
};

TEST_F(TezosMakeTransaction, CreateTx) {
    auto builder = tx_builder();

    auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED);

        getTestExecutionContext()->stop();
    });

    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);

    getTestExecutionContext()->waitUntilStopped();

    builder->setFees(api::Amount::fromLong(currency, 250));
    builder->setGasLimit(api::Amount::fromLong(currency, 10000));
    builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("1000")));
    // Self-transaction not allowed
    EXPECT_THROW(builder->sendToAddress(api::Amount::fromLong(currency, 220000), "tz1cmN7N6rV9ULVqbL2BxSUZgeL5wnWyoBUE"), Exception);
    builder->wipeToAddress("tz1TRspM5SeZpaQUhzByXbEvqKF1vnCM2YTK");
    // TODO: activate when we got URL of our custom explorer
    auto f = builder->build();
    auto tx = uv::wait(f);
    auto serializedTx = tx->serialize();
    auto balance = uv::wait(account->getBalance());
    EXPECT_EQ(balance->toLong(), tx->getValue()->toLong() + tx->getFees()->toLong());
    auto parsedTx = TezosLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedTx, "");
    auto serializedParsedTx = parsedTx->serialize();
    EXPECT_EQ(serializedTx, serializedParsedTx);

    auto date = "2000-03-27T09:10:22Z";
    auto formatedDate = DateUtils::fromJSON(date);

    //Delete account
    auto code = uv::wait(wallet->eraseDataSince(formatedDate));
    EXPECT_EQ(code, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

    //Check if account was successfully deleted
    auto newAccountCount = uv::wait(wallet->getAccountCount());
    EXPECT_EQ(newAccountCount, 0);
    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        TezosLikeAccountDatabaseEntry entry;
        auto result = TezosLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
        EXPECT_EQ(result, false);
    }

    auto originatedAccounts = account->getOriginatedAccounts();
    EXPECT_GT(originatedAccounts.size(), 0);

    std::cout << originatedAccounts.size() << " originated accounts." << std::endl;

    auto txBuilder = std::dynamic_pointer_cast<TezosLikeTransactionBuilder>(originatedAccounts[0]->buildTransaction());
    txBuilder->setFees(api::Amount::fromLong(currency, 250));
    txBuilder->setGasLimit(api::Amount::fromLong(currency, 10000));
    txBuilder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("1000")));
    txBuilder->sendToAddress(api::Amount::fromLong(currency, 220000), "tz1cmN7N6rV9ULVqbL2BxSUZgeL5wnWyoBUE");
    auto originatedTx = uv::wait(txBuilder->build());
    EXPECT_EQ(originatedTx->getSender()->toBase58(), "KT1JLbEZuWFhEyHXtKsvbCNZABXGehkjVyCd");
    EXPECT_EQ(originatedTx->getReceiver()->toBase58(), "tz1cmN7N6rV9ULVqbL2BxSUZgeL5wnWyoBUE");

    auto serializedOriginatedTx = originatedTx->serialize();
    // Remains deactivated because we don't parse KT Txs after Babylon Update
    //auto parsedOriginatedTx = TezosLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedOriginatedTx);
    //EXPECT_EQ(serializedOriginatedTx, parsedOriginatedTx->serialize());
    //Delete wallet
    auto walletCode = uv::wait(pool->eraseDataSince(formatedDate));
    EXPECT_EQ(walletCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

    //Check if wallet was successfully deleted
    auto walletCount = uv::wait(pool->getWalletCount());
    EXPECT_EQ(walletCount, 0);
}

TEST_F(TezosMakeTransaction, ParseUnsignedRawTransaction) {
    // round-trip
    auto strTx = "032cd54a6d49c82da8807044a41f8670cf31832cb12c151fe2f605407bcdf558960800008bd703c4a2d91b8f1d79455be9b99c2693e931fdfa0901d84f950280c8afa0250000d2e495a7ab40156d0a7c35b73d2530a3470fc87000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(ledger::core::currencies::TEZOS, txBytes, "");

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    //EXPECT_EQ(tx->getHash(), "opBQwjhyCEcH5JxUtthPxh7m7sBxGKnCzVYwUcR7xr7MGwotZnW");
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1YPSCGWXwBdTncK2aCctSZAXWvGsGwVJqU");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 10000000000L);
    EXPECT_EQ(tx->getFees()->toLong(), 1274L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10200L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "277");
}

TEST_F(TezosMakeTransaction, ParseUnsignedRawRevealTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94070000d2e495a7ab40156d0a7c35b73d2530a3470fc870ea0902904e0000cda3081bd81219ec494b29068dcfd19e427fed9a66abcdc9e9e99ca6478f60e9";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(ledger::core::currencies::TEZOS, txBytes, "");

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1258L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10000L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(TezosMakeTransaction, ParseUnsignedRawOriginationTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94090000d2e495a7ab40156d0a7c35b73d2530a3470fc870920903f44e950200d2e495a7ab40156d0a7c35b73d2530a3470fc8708094ebdc03ffff0000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(ledger::core::currencies::TEZOS, txBytes, "");

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1170L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10100L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "277");
}

TEST_F(TezosMakeTransaction, ParseUnsignedRawDelegationTransaction) {
    // round-trip
    auto strTx = "037d8d230a91d1fb8391727f37a1bfeb332b7f249c78315ea4ae934e2103a826630a01d315f72434520d43d415f0dff4632519501d2d9400890902f44e00ff008bd703c4a2d91b8f1d79455be9b99c2693e931fd";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(ledger::core::currencies::TEZOS, txBytes, "");

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
TEST_F(TezosMakeTransaction, ParseSignedRawDelegationTransaction) {
    // round-trip
    auto strTx = "4fd4dca725498e819cc4dd6a87adcfb98770f600558d5d097d1a48b2324a9a2e080000bbdd4268871d1751a601fe66603324714266bf558c0bdeff4ebc50ac02a08d0601583f106387cb85212812b738cae45b497551bf9a00007dc21f46b94d6b432c881b78e1fee917ef0fd382571a5d5f1bf1c5aa90d62a02777eeebac53e3fe9bbcf4501cc2ee0cb1dbe65ec24c869a4715d84f65cfdc101";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawSignedTransaction(ledger::core::currencies::TEZOS, txBytes, "");

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
