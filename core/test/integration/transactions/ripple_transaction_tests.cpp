/*
 *
 * ripple_transaction_tests
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
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
#include "../../fixtures/xrp_fixtures.h"
#include <api/KeychainEngines.hpp>
#include "transaction_test_helper.h"
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/ripple/database/RippleLikeAccountDatabaseHelper.h>
#include <wallet/ripple/api_impl/RippleLikeTransactionApi.h>
#include <wallet/currencies.hpp>
#include <iostream>
using namespace std;

struct RippleMakeTransaction : public RippleMakeBaseTransaction {
    void SetUpConfig() override {
        auto configuration = DynamicObject::newInstance();
        testData.configuration = configuration;
        testData.walletName = "my_wallet";
        testData.currencyName = "ripple";
        testData.inflate_xrp = ledger::testing::xrp::inflate;
    }
};

TEST_F(RippleMakeTransaction, CreateTx) {
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
        auto txBuilder = std::dynamic_pointer_cast<RippleLikeTransactionBuilder>(account->buildTransaction());
        dispatcher->stop();
    });

    account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);

    dispatcher->waitUntilStopped();

    auto balance = wait(account->getBalance());
    auto fromDate = "2018-01-01T13:38:23Z";
    auto toDate = DateUtils::toJSON(DateUtils::now());
    auto balanceHistory = wait(account->getBalanceHistory(fromDate, toDate, api::TimePeriod::MONTH));

    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), balance->toLong());

    builder->setFees(api::Amount::fromLong(currency, 10));
    builder->sendToAddress(api::Amount::fromLong(currency, 220000), "rMspb4Kxa3EwdF4uN5TMqhHfsAkBit6w7k");
    auto f = builder->build();
    auto tx = ::wait(f);
    auto serializedTx = tx->serialize();
    auto parsedTx = RippleLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedTx);
    auto serializedParsedTx = parsedTx->serialize();
    EXPECT_EQ(serializedTx, serializedParsedTx);

    auto date = "2000-03-27T09:10:22Z";
    auto formatedDate = DateUtils::fromJSON(date);

    //Delete account
    EXPECT_NO_THROW(stlab::blocking_get(wallet->eraseDataSince(formatedDate)));
    
    //Check if account was successfully deleted
    auto newAccountCount = wait(wallet->getAccountCount());
    EXPECT_EQ(newAccountCount, 0);
    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        RippleLikeAccountDatabaseEntry entry;
        auto result = RippleLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
        EXPECT_EQ(result, false);
    }

    //Delete wallet
    EXPECT_NO_THROW(stlab::blocking_get(pool->eraseDataSince(formatedDate)));
    
    //Check if wallet was successfully deleted
    auto walletCount = stlab::blocking_get(pool->getWalletCount());
    EXPECT_EQ(walletCount, 0);

}


TEST_F(RippleMakeTransaction, ParseSignedRawTransaction) {
    //Tx hash af4bb95de86a640b90b2af3c696ef26efe7dd71864cc959d8030b448dd48e756
    auto strTx = "12000022800000002400000001201B02A2618F6140000000014FB18068400000000000000A73210215A9EE08A4B4747E27F348365F93BEB5897FA7E8776BEDAE2CB56917DCDBBF2F74473045022100F2AB61EC941462D692514BFDDB00BC0D31BA7DA66981193E67A04E90578C18B1022064A2375ECB5A68C22EE3038B783BE6A9E1F2C882A8E8BBEE43C4CFA93B536926811420237754A1727016188A1B7E52F2060F94339D128314DBC4AD4F38B60FA5624D0DDEDDAC209BABBAA9D7";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::RippleLikeTransactionBuilder::parseRawSignedTransaction(ledger::core::currencies::RIPPLE, txBytes);
    EXPECT_EQ(tx->getHash(), "af4bb95de86a640b90b2af3c696ef26efe7dd71864cc959d8030b448dd48e756");
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
}