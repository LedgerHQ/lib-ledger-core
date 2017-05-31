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

#include <gtest/gtest.h>
#include <async/QtThreadDispatcher.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <NativePathResolver.hpp>
#include <unordered_set>
#include <src/wallet/pool/WalletPool.hpp>
#include <CoutLogPrinter.hpp>
#include <src/api/DynamicObject.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>

using namespace ledger::core;
using namespace ledger::qt;

static const std::string XPUB_1 = "xpub6EedcbfDs3pkzgqvoRxTW6P8NcCSaVbMQsb6xwCdEBzqZBronwY3Nte1Vjunza8f6eSMrYvbM5CMihGo6SbzpHxn4R5pvcr2ZbZ6wkDmgpy";

static void createWallet(const std::shared_ptr<WalletPool>& pool, const std::string& walletName) {
    soci::session sql(pool->getDatabaseSessionPool()
                          ->getPool());
    WalletDatabaseEntry entry;
    entry.configuration = std::static_pointer_cast<DynamicObject>(DynamicObject::newInstance());
    entry.name = "my_wallet";
    entry.poolName = pool->getName();
    entry.currencyName = "bitcoin";
    entry.updateUid();
    PoolDatabaseHelper::putWallet(sql, entry);
}

static void createAccount(const std::shared_ptr<WalletPool>& pool, const std::string& walletName, int32_t index) {
    soci::session sql(pool->getDatabaseSessionPool()
                          ->getPool());
    auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), walletName, "bitcoin");
    if (!AccountDatabaseHelper::accountExists(sql, walletUid, index))
        AccountDatabaseHelper::createAccount(sql, walletUid, index);
}

static BitcoinLikeWalletDatabase newAccount(const std::shared_ptr<WalletPool>& pool,
                                            const std::string& walletName,
                                            int32_t index,
                                            const std::string& xpub) {
    BitcoinLikeWalletDatabase db(pool, walletName, "bitcoin");
    if (!db.accountExists(index)) {
        createWallet(pool, walletName);
        createAccount(pool, walletName, index);
        db.createAccount(index, xpub);
    }
    return db;
}

TEST(BitcoinWalletDatabase, EmptyWallet) {
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

TEST(BitcoinWalletDatabase, CreateWalletWithOneAccount) {
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

TEST(BitcoinWalletDatabase, CreateWalletWithMultipleAccountAndDelete) {
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