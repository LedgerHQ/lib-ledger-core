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
    auto balance = uv::wait(account->getBalance());
    auto fromDate = "2018-01-01T13:38:23Z";
    auto toDate = DateUtils::toJSON(DateUtils::now());
    auto balanceHistory = uv::wait(account->getBalanceHistory(fromDate, toDate, api::TimePeriod::MONTH));

    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), balance->toLong());

    builder->setGasPrice(api::Amount::fromLong(currency, 200000));
    builder->setGasLimit(api::Amount::fromLong(currency, 20000000));
    builder->sendToAddress(api::Amount::fromLong(currency, 200000), "0xfb98bdd04d82648f25e67041d6e27a866bec0b47");
    auto f = builder->build();
    EXPECT_THROW(uv::wait(f), ledger::core::Exception);
    builder->setGasPrice(api::Amount::fromLong(currency, 0));
    builder->setGasLimit(api::Amount::fromLong(currency, 0));
    builder->sendToAddress(api::Amount::fromLong(currency, 0), "0xfb98bdd04d82648f25e67041d6e27a866bec0b47");
    f = builder->build();
    auto tx = uv::wait(f);
    auto addressList = uv::wait(account->getFreshPublicAddresses());
    EXPECT_EQ(addressList.size(), 1);
    EXPECT_EQ(addressList[0]->toString(), tx->getSender()->toEIP55());
    auto serializedTx = tx->serialize();
    auto parsedTx = EthereumLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedTx);
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
        EthereumLikeAccountDatabaseEntry entry;
        auto result = EthereumLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
        EXPECT_EQ(result, false);
    }

    //Delete wallet
    auto walletCode = uv::wait(pool->eraseDataSince(formatedDate));
    EXPECT_EQ(walletCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

    //Check if wallet was successfully deleted
    auto walletCount = uv::wait(pool->getWalletCount());
    EXPECT_EQ(walletCount, 0);

}

TEST_F(EthereumMakeTransaction, ParseUnsignedRawTransaction) {
    //Tx hash 4858a0a3d5f1de0c0f5729f25c3501bda946093aed07f842e53a90ac65d66f70
    auto strTx = "e880850165a0bc0083030d4094a49386fff4e0dd767b145e75d92f7fba8854553e8301e0f380018080";
    auto tx = api::EthereumLikeTransactionBuilder::parseRawUnsignedTransaction(ledger::core::currencies::ETHEREUM, hex::toByteArray(strTx));
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
}

TEST_F(EthereumMakeTransaction, ParseSignedRawTransactionAndBuildETHBlockchainExplorerTx) {
    //Tx hash 71df2bc7d800a05a97b742aa5313e5040003ba3ddb84546ceb2a026e5a97fca5 (Ropsten)
    auto txHash = "71df2bc7d800a05a97b742aa5313e5040003ba3ddb84546ceb2a026e5a97fca5";
    auto rawTx = "f8678183843b9aca0083030d4094ac6603e97e774cd34603293b69bbbb1980aceeaa8201008029a0c4cbbdbdecb855505cd344dd1b800e1cce731ebc0f9a1559f0bc9da5ab20daa6a01adbe3788efac90885117f23bd8cfaa859e4f1afbd1416087b4172aca3a0423f";
    auto ethLikeBCTx = EthereumLikeAccount::getETHLikeBlockchainExplorerTxFromRawTx(account, txHash, hex::toByteArray(rawTx));
    EXPECT_EQ(ethLikeBCTx.status, 1);
    // For test purposes sender is account's address (real sender is 0xd0ec064cff693453ef4595aa555ce65244b212a5)
    auto sender = account->getKeychain()->getAddress()->toEIP55();
    auto receiver = "0xAc6603e97e774Cd34603293b69bBBB1980acEeaA";
    EXPECT_EQ(ethLikeBCTx.sender,  sender);
    EXPECT_EQ(ethLikeBCTx.receiver, receiver);
    EXPECT_EQ(ethLikeBCTx.value.toString(), "256");
    EXPECT_EQ(ethLikeBCTx.gasPrice.toString(), "1000000000");
    EXPECT_EQ(ethLikeBCTx.gasLimit.toString(), "200000");
    EXPECT_EQ(ethLikeBCTx.inputData.size(), 0);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions.size(), 0);
}

TEST_F(EthereumMakeTransaction, ParseSignedRawERC20TransactionAndBuildETHBlockchainExplorerTx) {
    //Tx hash 96c71eac2350d498ba89863465c1c362fc3ec323f33b5eb65ee60da5cb75420a (Ropsten)
    auto txHash = "96c71eac2350d498ba89863465c1c362fc3ec323f33b5eb65ee60da5cb75420a";
    auto rawTx = "f8aa8180843b9aca0083030d4094dfb287530fd4c1e59456de82a84e4aae7c250ec180b844a9059cbb000000000000000000000000456b8e57f5e096b9fff45bdbd58b8ce90d830ff900000000000000000000000000000000000000000000001b1ae4d6e2ef50000029a0fde02005f6bb452269dabb128ad2c9a44090f6f160fc7be8ef511224524fc5b9a06e07d22f5e92435dfcfc633f85825291c118d0f05ef46ea71774f306325d7ea2";
    auto ethLikeBCTx = EthereumLikeAccount::getETHLikeBlockchainExplorerTxFromRawTx(account, txHash, hex::toByteArray(rawTx));
    EXPECT_EQ(ethLikeBCTx.status, 1);
    // For test purposes sender is account's address (real sender is 0xd0ec064cff693453ef4595aa555ce65244b212a5)
    auto sender = account->getKeychain()->getAddress()->toEIP55();
    auto receiver = "0x456b8e57F5e096B9Fff45BDbD58B8CE90d830Ff9";
    auto contractAddress = "0xDFb287530FD4c1e59456DE82a84e4aae7C250Ec1";
    EXPECT_EQ(ethLikeBCTx.sender, sender);
    EXPECT_EQ(ethLikeBCTx.receiver, contractAddress);
    EXPECT_EQ(ethLikeBCTx.value, BigInt::ZERO);
    EXPECT_EQ(ethLikeBCTx.gasPrice.toString(), "1000000000");
    EXPECT_EQ(ethLikeBCTx.gasLimit.toString(), "200000");
    EXPECT_EQ(hex::toString(ethLikeBCTx.inputData), "a9059cbb000000000000000000000000456b8e57f5e096b9fff45bdbd58b8ce90d830ff900000000000000000000000000000000000000000000001b1ae4d6e2ef500000");
    EXPECT_EQ(ethLikeBCTx.erc20Transactions.size(), 1);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].value.toString(), "500000000000000000000");
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].from, sender);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].to, receiver);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].contractAddress, contractAddress);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].type, api::OperationType::SEND);
}