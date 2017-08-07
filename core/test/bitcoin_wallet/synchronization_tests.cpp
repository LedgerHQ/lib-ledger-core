/*
 *
 * synchronization_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/07/2017.
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
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <async/async_wait.h>
#include <BitcoinLikeStringXpubProvider.h>
#include <api/BitcoinLikeExtendedPublicKeyProvider.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <api/BitcoinLikeOperation.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BigInt.hpp>
#include <net/QtHttpClient.hpp>
#include <events/LambdaEventReceiver.hpp>

using namespace ledger::core;
using namespace ledger::qt;


#include "../fixtures/fixtures_1.h"

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


TEST(BitcoinLikeWalletSynchronization, MediumXpubSynchronization) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto http = std::make_shared<QtHttpClient>(dispatcher->getMainExecutionContext());
    auto newPool = [=]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                http,
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

        account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), make_receiver([=] (const std::shared_ptr<api::Event>& event) {
            fmt::print("Received event {}\n", api::to_string(event->getCode()));
            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                return ;
            EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
            if (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT) {

            }
            dispatcher->stop();
        }));

        dispatcher->waitUntilStopped();
    }
    resolver->clean();
}