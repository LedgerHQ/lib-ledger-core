/*
 * AlgorandDatabaseTests
 *
 * Created by Hakim Aammar on 25/05/2020.
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

#include "AlgorandTestFixtures.hpp"
#include <algorand/database/AlgorandTransactionDatabaseHelper.hpp>
#include <algorand/database/AlgorandAccountDatabaseHelper.hpp>
#include <algorand/operations/AlgorandOperation.hpp>
#include <algorand/AlgorandWalletFactory.hpp>
#include <algorand/AlgorandLikeCurrencies.hpp>
#include <algorand/AlgorandWallet.hpp>
#include <algorand/AlgorandAccount.hpp>

#include <core/operation/OperationQuery.hpp>
#include <core/api/AccountCreationInfo.hpp>
#include <core/api/Address.hpp>

#include <integration/WalletFixture.hpp>

#include <utility>

using namespace ledger::testing::algorand;
using namespace ledger::core::algorand;

class AlgorandDatabaseTest : public WalletFixture<WalletFactory> {

    public:

    void SetUp() override {
        WalletFixture::SetUp();

        auto const currency = currencies::algorand();
        registerCurrency(currency);

        accountInfo = api::AccountCreationInfo(1, {}, {}, { algorand::Address::toPublicKey(OBELIX_ADDRESS) }, {});

        auto wallet = wait(walletStore->createWallet("algorand", currency.name, api::DynamicObject::newInstance()));
        auto account = createAccount<Account>(wallet, accountInfo.index, accountInfo);

        accountUid = algorand::AccountDatabaseHelper::createAccountUid(wallet->getWalletUid(), accountInfo.index);
    }

    void TearDown() override {
        WalletFixture::TearDown();

        wallet.reset();
        account.reset();
    }

    std::shared_ptr<Wallet> wallet;
    std::shared_ptr<Account> account;

    api::AccountCreationInfo accountInfo;
    std::string accountUid;
};

TEST_F(AlgorandDatabaseTest, AccountDBTest) {

    // Test reading from DB
    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());

        algorand::AccountDatabaseEntry accountFromDB;
        auto result = algorand::AccountDatabaseHelper::queryAccount(sql, accountUid, accountFromDB);

        EXPECT_EQ(result, true);
        EXPECT_EQ(accountInfo.index, accountFromDB.index);
        EXPECT_EQ(OBELIX_ADDRESS, accountFromDB.address) << "This test fails because SHA512-256 is not implemented yet!";
    }
}

TEST_F(AlgorandDatabaseTest, TransactionsDBTest) {

    auto paymentTxRef = paymentTransaction();
    auto assetConfigTxRef = assetConfigTransaction();
    auto assetTransferTxRef = assetTransferTransaction();

    // Test writing into DB
    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());
        algorand::TransactionDatabaseHelper::putTransaction(sql, "test-account", paymentTxRef);
        algorand::TransactionDatabaseHelper::putTransaction(sql, "test-account", assetConfigTxRef);
        algorand::TransactionDatabaseHelper::putTransaction(sql, "test-account", assetTransferTxRef);
    }

    // Test reading from DB
    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());

        model::Transaction paymentTxFromDB;
        auto result = algorand::TransactionDatabaseHelper::getTransactionByHash(sql, *paymentTxRef.header.id, paymentTxFromDB);
        EXPECT_TRUE(result);
        assertSameTransaction(paymentTxRef, paymentTxFromDB);

        model::Transaction assetConfigTxFromDB;
        result = algorand::TransactionDatabaseHelper::getTransactionByHash(sql, *assetConfigTxRef.header.id, assetConfigTxFromDB);
        EXPECT_TRUE(result);
        assertSameTransaction(assetConfigTxRef, assetConfigTxFromDB);

        model::Transaction assetTransferTxFromDB;
        result = algorand::TransactionDatabaseHelper::getTransactionByHash(sql, *assetTransferTxRef.header.id, assetTransferTxFromDB);
        EXPECT_TRUE(result);
        assertSameTransaction(assetTransferTxRef, assetTransferTxFromDB);
    }
}

TEST_F(AlgorandDatabaseTest, OperationsDBTest) {

    auto txRef = paymentTransaction();

    // Test writing into DB
    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());
        account->putTransaction(sql, txRef);
    }

    // Test reading from DB
    {
        auto ops = wait(std::dynamic_pointer_cast<OperationQuery<algorand::Operation>>(account->queryOperations()->complete())->execute());

        EXPECT_EQ(ops.size(), 1);
        auto op = std::dynamic_pointer_cast<algorand::Operation>(ops[0]);

        /* TODO ?
        EXPECT_EQ(op->getAccountIndex(), account->getIndex());
        EXPECT_EQ(op->getAmount(), ?);
        EXPECT_EQ(op->getBlockHeight(), ?);
        EXPECT_EQ(op->getCurrency(), ?);
        EXPECT_EQ(op->getDate(), ?);
        EXPECT_EQ(op->getFees(), ?);
        EXPECT_EQ(op->getOperationType(), ?);
        EXPECT_EQ(op->getPreferences(), ?);
        EXPECT_EQ(op->getRecipients(), ?);
        EXPECT_EQ(op->getSenders(), ?);
        EXPECT_EQ(op->getTrust(), ?);
        EXPECT_EQ(op->getUid(), ?);
        EXPECT_EQ(op->isComplete(), ?);
        */

        auto txRetrieved = op->getTransactionData();
        assertSameTransaction(txRef, txRetrieved);
    }
}