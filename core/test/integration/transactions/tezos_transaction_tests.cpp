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
        testData.configuration = configuration;
        testData.walletName = "my_wallet";
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

        auto balance = wait(account->getBalance());
        std::cout << "Balance: " << balance->toString() << std::endl;
        auto txBuilder = std::dynamic_pointer_cast<TezosLikeTransactionBuilder>(account->buildTransaction());
        dispatcher->stop();
    });

    account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);

    dispatcher->waitUntilStopped();

    auto balance = wait(account->getBalance());
    auto fromDate = "2018-01-01T13:38:23Z";
    auto toDate = DateUtils::toJSON(DateUtils::now());
    auto balanceHistory = wait(account->getBalanceHistory(fromDate, toDate, api::TimePeriod::MONTH));

    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), balance->toLong());

    builder->setFees(api::Amount::fromLong(currency, 250));
    builder->setGasLimit(api::Amount::fromLong(currency, 10000));
    builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("1000")));
    builder->sendToAddress(api::Amount::fromLong(currency, 220000), "tz1TRspM5SeZpaQUhzByXbEvqKF1vnCM2YTK");
    auto f = builder->build();
    auto tx = ::wait(f);
    auto serializedTx = tx->serialize();
    auto parsedTx = TezosLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedTx);
    auto serializedParsedTx = parsedTx->serialize();
    EXPECT_EQ(serializedTx, serializedParsedTx);

    auto date = "2000-03-27T09:10:22Z";
    auto formatedDate = DateUtils::fromJSON(date);

    //Delete account
    auto code = wait(wallet->eraseDataSince(formatedDate));
    EXPECT_EQ(code, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

    //Check if account was successfully deleted
    auto newAccountCount = wait(wallet->getAccountCount());
    EXPECT_EQ(newAccountCount, 0);
    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        TezosLikeAccountDatabaseEntry entry;
        auto result = TezosLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
        EXPECT_EQ(result, false);
    }

    //Delete wallet
    auto walletCode = wait(pool->eraseDataSince(formatedDate));
    EXPECT_EQ(walletCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

    //Check if wallet was successfully deleted
    auto walletCount = wait(pool->getWalletCount());
    EXPECT_EQ(walletCount, 0);

}

TEST_F(TezosMakeTransaction, ParseSignedRawTransaction) {
    // round-trip
    auto strTx = "032cd54a6d49c82da8807044a41f8670cf31832cb12c151fe2f605407bcdf558960800018bd703c4a2d91b8f1d79455be9b99c2693e931fdfa0901d84f950280c8afa0250001d2e495a7ab40156d0a7c35b73d2530a3470fc87000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawSignedTransaction(ledger::core::currencies::TEZOS, txBytes);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    //EXPECT_EQ(tx->getHash(), "opBQwjhyCEcH5JxUtthPxh7m7sBxGKnCzVYwUcR7xr7MGwotZnW");
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1YPSCGWXwBdTncK2aCctSZAXWvGsGwVJqU");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 10000000000L);
    EXPECT_EQ(tx->getFees()->toLong(), 1274L);
}

TEST_F(TezosMakeTransaction, ParseSignedRawRevealTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94070000d2e495a7ab40156d0a7c35b73d2530a3470fc870ea0902904e0000cda3081bd81219ec494b29068dcfd19e427fed9a66abcdc9e9e99ca6478f60e9";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawSignedTransaction(ledger::core::currencies::TEZOS, txBytes);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1258L);
}

TEST_F(TezosMakeTransaction, ParseSignedRawOriginationTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94090000d2e495a7ab40156d0a7c35b73d2530a3470fc870920903f44e950200d2e495a7ab40156d0a7c35b73d2530a3470fc8708094ebdc03ffff0000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawSignedTransaction(ledger::core::currencies::TEZOS, txBytes);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1258L);
}