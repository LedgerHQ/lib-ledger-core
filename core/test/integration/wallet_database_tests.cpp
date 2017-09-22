/*
 *
 * wallet_database_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/05/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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

#define TEST_SUITE_NAME BitcoinWalletDatabase

#include "../fixtures/fixtures_1.h"
#include "../fixtures/bitcoin_helpers.h"

TEST_F(BitcoinWalletDatabase, EmptyWallet) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();
        BitcoinLikeWalletDatabase db(pool, "my_wallet", "bitcoin");

        EXPECT_EQ(db.getAccountsCount(), 0);
        EXPECT_FALSE(db.accountExists(255));
    }
    resolver->clean();
}

TEST_F(BitcoinWalletDatabase, CreateWalletWithOneAccount) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db(pool, "my_wallet", "bitcoin");

        EXPECT_EQ(db.getAccountsCount(), 0);
        EXPECT_FALSE(db.accountExists(0));

        // We need to create the abstract entry first to satisfy the foreign key constraint
        createWallet(pool, "my_wallet");
        createAccount(pool, "my_wallet", 0);

        db.createAccount(0, XPUB_1);

        EXPECT_EQ(db.getAccountsCount(), 1);
        EXPECT_TRUE(db.accountExists(0));
    }
    resolver->clean();
}

TEST_F(BitcoinWalletDatabase, CreateWalletWithMultipleAccountAndDelete) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db = newAccount(pool, "my_wallet", 0, XPUB_1);

        EXPECT_EQ(db.getAccountsCount(), 1);
        EXPECT_EQ(db.getNextAccountIndex(), 1);
        for (auto i = 1; i < 100; i++) {
            newAccount(pool, "my_wallet", i, XPUB_1);
        }
        EXPECT_EQ(db.getAccountsCount(), 100);

        soci::session sql(pool->getDatabaseSessionPool()->getPool());

        auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), "my_wallet", "bitcoin");
        EXPECT_EQ(AccountDatabaseHelper::getAccountsCount(sql, walletUid), 100);
        AccountDatabaseHelper::removeAccount(sql, walletUid, 0);
        EXPECT_EQ(AccountDatabaseHelper::getAccountsCount(sql, walletUid), 99);
        EXPECT_EQ(db.getAccountsCount(), 99);
        EXPECT_EQ(db.getNextAccountIndex(), 0);
    }
    resolver->clean();
}

TEST_F(BitcoinWalletDatabase, PutTransaction) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db = newAccount(pool, "my_wallet", 0, XPUB_1);
        auto transaction = JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION);
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        BitcoinLikeAccountDatabase acc(db.getWalletUid(), 0);
        BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, *transaction);

        BitcoinLikeBlockchainExplorer::Transaction dbTransaction;
        if (BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, transaction->hash, dbTransaction)) {
            EXPECT_EQ(transaction->hash, dbTransaction.hash);
            EXPECT_EQ(transaction->lockTime, dbTransaction.lockTime);
            EXPECT_EQ(transaction->receivedAt, dbTransaction.receivedAt);
            EXPECT_EQ(transaction->inputs[0].value.getValue().toUint64(), 2194550UL);
            EXPECT_EQ(transaction->inputs[0].address.getValue(), dbTransaction.inputs[0].address.getValue());
            EXPECT_EQ(transaction->outputs[0].value.toUint64(), dbTransaction.outputs[0].value.toUint64());
            EXPECT_EQ(transaction->outputs[0].address.getValue(), dbTransaction.outputs[0].address.getValue());
        } else {
            FAIL();
        }

    }
    resolver->clean();
}

TEST_F(BitcoinWalletDatabase, PutTransactionWithMultipleOutputs) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db = newAccount(pool, "my_wallet", 0, XPUB_1);
        std::vector<BitcoinLikeBlockchainExplorer::Transaction> transactions = {
                *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION),
                *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION_2),
                *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION_3)
        };
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.begin();
        BitcoinLikeAccountDatabase acc(db.getWalletUid(), 0);
        for (auto& transaction : transactions) {
            BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, transaction);
        }
        sql.commit();

        for (auto& transaction : transactions) {
            BitcoinLikeBlockchainExplorer::Transaction dbTx;
            if (BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, transaction.hash, dbTx)) {
                EXPECT_EQ(transaction.hash, dbTx.hash);
                EXPECT_EQ(transaction.lockTime, dbTx.lockTime);
                EXPECT_EQ(transaction.receivedAt.time_since_epoch().count(), dbTx.receivedAt.time_since_epoch().count());
                EXPECT_EQ(transaction.block.isEmpty(), dbTx.block.isEmpty());
                if (transaction.block.nonEmpty()) {
                    auto& block = transaction.block.getValue();
                    auto& dbBlock = dbTx.block.getValue();
                    EXPECT_EQ(block.hash, dbBlock.hash);
                    EXPECT_EQ(block.height, dbBlock.height);
                    EXPECT_EQ(block.time.time_since_epoch().count(), dbBlock.time.time_since_epoch().count());
                }
                EXPECT_EQ(transaction.inputs.size(), dbTx.inputs.size());
                for (auto i = 0; i < transaction.inputs.size(); i++) {
                    auto& input = transaction.inputs[i];
                    auto& dbInput = dbTx.inputs[i];
                    EXPECT_EQ(input.address.getValueOr(""), dbInput.address.getValueOr(""));
                    EXPECT_EQ(input.coinbase.getValueOr(""), dbInput.coinbase.getValueOr(""));
                    EXPECT_EQ(input.index, dbInput.index);
                    EXPECT_EQ(input.previousTxHash.getValueOr(""), dbInput.previousTxHash.getValueOr(""));
                    EXPECT_EQ(input.previousTxOutputIndex.getValueOr(0), dbInput.previousTxOutputIndex.getValueOr(0));
                    EXPECT_EQ(input.sequence, dbInput.sequence);
                    EXPECT_EQ(input.value.getValue().toUint64(), dbInput.value.getValue().toUint64());
                }
                EXPECT_EQ(transaction.outputs.size(), dbTx.outputs.size());
                for (auto i = 0; i < transaction.outputs.size(); i++) {
                    auto& output = transaction.outputs[i];
                    auto& dbOutput = dbTx.outputs[i];
                    EXPECT_EQ(output.address.getValueOr(""), dbOutput.address.getValueOr(""));
                    EXPECT_EQ(output.value.toUint64(), dbOutput.value.toUint64());
                    EXPECT_EQ(output.index, dbOutput.index);
                    EXPECT_EQ(output.script, dbOutput.script);
                }
            } else {
                FAIL();
            }
        }
    }
    resolver->clean();
}

TEST_F(BitcoinWalletDatabase, PutOperations) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();
        auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
        auto nextIndex = wait(wallet->getNextAccountIndex());
        EXPECT_EQ(nextIndex, 0);
        auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(std::dynamic_pointer_cast<BitcoinLikeWallet>(wallet->asBitcoinLikeWallet())->createNewAccount(nextIndex, XPUB_PROVIDER)));

        std::vector<BitcoinLikeBlockchainExplorer::Transaction> transactions = {
                *JSONUtils::parse<TransactionParser>(TX_1),
                *JSONUtils::parse<TransactionParser>(TX_2),
                *JSONUtils::parse<TransactionParser>(TX_3),
                *JSONUtils::parse<TransactionParser>(TX_4)
        };
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.begin();
        for (auto& tx : transactions) {
            account->putTransaction(sql, tx);
        }
        sql.commit();
        auto query = account->queryOperations()->complete()->addOrder(api::OperationOrderKey::DATE, false)->addOrder(api::OperationOrderKey::TYPE, false);
        auto operations = wait(std::static_pointer_cast<OperationQuery>(query)->execute());
        EXPECT_EQ(operations.size(), 5);

        auto expectation_0 = std::make_tuple("666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5", 15, 1,
                                             890000, "1KMbwcH1sGpHetLwwQVNMt4cEZB5u8Uk4b", 182593500, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7",
                                             182593500, api::OperationType::RECEIVE, 362035);
        auto expectation_1 = std::make_tuple("a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc", 1, 2,
                                             182593500, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7", 182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1",
                                             100000, api::OperationType::RECEIVE, 362055);

        auto expectation_2 = std::make_tuple("a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc", 1, 2,
                                             182593500, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7", 182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1",
                                             100000, api::OperationType::SEND, 362055);

        auto expectation_3 = std::make_tuple("4450e70656888bd7f5240a9b532eac54db7d72f3b48bfef09fb45a185bb9c570", 1, 2,
                                             182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1", 182373500, "139dJmHhFuuhrgbNAPehpokjYHNEtvkxot",
                                             100000, api::OperationType::RECEIVE, 362058);

        auto expectation_4 = std::make_tuple("4450e70656888bd7f5240a9b532eac54db7d72f3b48bfef09fb45a185bb9c570", 1, 2,
                                             182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1", 182373500, "139dJmHhFuuhrgbNAPehpokjYHNEtvkxot",
                                             100000, api::OperationType::SEND, 362058);

        #define ASSERT_EXPECTATION(it) \
        { \
            auto& op = operations[it]; \
            auto tx = op->asBitcoinLikeOperation()->getTransaction(); \
            auto& expect = expectation_##it; \
            EXPECT_EQ(tx->getHash(), std::get<0>(expect)); \
            EXPECT_EQ(tx->getInputs().size(), std::get<1>(expect)); \
            EXPECT_EQ(tx->getOutputs().size(), std::get<2>(expect)); \
            EXPECT_EQ(tx->getInputs()[0]->getValue()->toBigInt()->intValue(), std::get<3>(expect)); \
            EXPECT_EQ(tx->getInputs()[0]->getAddress().value(), std::get<4>(expect)); \
            EXPECT_EQ(tx->getOutputs()[0]->getValue()->toBigInt()->intValue(), std::get<5>(expect)); \
            EXPECT_EQ(tx->getOutputs()[0]->getAddress().value(), std::get<6>(expect)); \
            EXPECT_EQ(op->getAmount()->toBigInt()->intValue(), std::get<7>(expect)); \
            EXPECT_EQ(op->getOperationType(), std::get<8>(expect)); \
            EXPECT_EQ(op->getBlockHeight().value(), std::get<9>(expect)); \
        }


        ASSERT_EXPECTATION(0);
        ASSERT_EXPECTATION(1);
        ASSERT_EXPECTATION(2);
        ASSERT_EXPECTATION(3);
        ASSERT_EXPECTATION(4);
    }
    resolver->clean();
}