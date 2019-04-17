/*
 *
 * ethereum_transaction_tests
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
#include "../../fixtures/eth_xpub_fixtures.h"
#include <api/KeychainEngines.hpp>
#include "transaction_test_helper.h"
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/ethereum/database/EthereumLikeAccountDatabaseHelper.h>
#include <wallet/ethereum/api_impl/EthereumLikeTransactionApi.h>
#include <wallet/currencies.hpp>
#include <iostream>
using namespace std;

struct EthereumMakeTransaction : public EthereumMakeBaseTransaction {
    void SetUpConfig() override {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION,"v2");
        testData.configuration = configuration;
        testData.walletName = "my_wallet";
        testData.currencyName = "ethereum";
        testData.inflate_eth = ledger::testing::eth_xpub::inflate;
    }
};

TEST_F(EthereumMakeTransaction, CreateStandardWithOneOutput) {
    auto builder = tx_builder();
    auto balance = wait(account->getBalance());
    auto fromDate = "2018-01-01T13:38:23Z";
    auto toDate = DateUtils::toJSON(DateUtils::now());
    auto balanceHistory = wait(account->getBalanceHistory(fromDate, toDate, api::TimePeriod::MONTH));

    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), balance->toLong());

    builder->setGasPrice(api::Amount::fromLong(currency, 200000));
    builder->setGasLimit(api::Amount::fromLong(currency, 20000000));
    builder->sendToAddress(api::Amount::fromLong(currency, 200000), "0xfb98bdd04d82648f25e67041d6e27a866bec0b47");
    auto f = builder->build();
    auto tx = ::wait(f);
    auto addressList = ::wait(account->getFreshPublicAddresses());
    EXPECT_EQ(addressList.size(), 1);
    EXPECT_EQ(addressList[0]->toString(), tx->getSender()->toEIP55());
    auto serializedTx = tx->serialize();
    auto parsedTx = EthereumLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedTx);
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
        EthereumLikeAccountDatabaseEntry entry;
        auto result = EthereumLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
        EXPECT_EQ(result, false);
    }

    //Delete wallet
    auto walletCode = wait(pool->eraseDataSince(formatedDate));
    EXPECT_EQ(walletCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

    //Check if wallet was successfully deleted
    auto walletCount = wait(pool->getWalletCount());
    EXPECT_EQ(walletCount, 0);

}

TEST_F(EthereumMakeTransaction, ParseUnsignedRawTransaction) {
    //Tx hash 4858a0a3d5f1de0c0f5729f25c3501bda946093aed07f842e53a90ac65d66f70
    auto strTx = "E800850165A0BC0083030D4094A49386FFF4E0DD767B145E75D92F7FBA8854553E8301E0F380018080";
    auto tx = api::EthereumLikeTransactionBuilder::parseRawUnsignedTransaction(ledger::core::currencies::ETHEREUM, hex::toByteArray(strTx));
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
}