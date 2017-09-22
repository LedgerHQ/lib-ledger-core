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
#include "BaseFixture.h"

class BitcoinLikeWalletSynchronization : public BaseFixture {

};
//
//static void createWallet(const std::shared_ptr<WalletPool>& pool, const std::string& walletName) {
//    soci::session sql(pool->getDatabaseSessionPool()
//                          ->getPool());
//    WalletDatabaseEntry entry;
//    entry.configuration = std::static_pointer_cast<DynamicObject>(DynamicObject::newInstance());
//    entry.name = "my_wallet";
//    entry.poolName = pool->getName();
//    entry.currencyName = "bitcoin";
//    entry.updateUid();
//    PoolDatabaseHelper::putWallet(sql, entry);
//}
//
//static void createAccount(const std::shared_ptr<WalletPool>& pool, const std::string& walletName, int32_t index) {
//    soci::session sql(pool->getDatabaseSessionPool()
//                          ->getPool());
//    auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), walletName, "bitcoin");
//    if (!AccountDatabaseHelper::accountExists(sql, walletUid, index))
//        AccountDatabaseHelper::createAccount(sql, walletUid, index);
//}
//
//static BitcoinLikeWalletDatabase newAccount(const std::shared_ptr<WalletPool>& pool,
//                                            const std::string& walletName,
//                                            int32_t index,
//                                            const std::string& xpub) {
//    BitcoinLikeWalletDatabase db(pool, walletName, "bitcoin");
//    if (!db.accountExists(index)) {
//        createWallet(pool, walletName);
//        createAccount(pool, walletName, index);
//        db.createAccount(index, xpub);
//    }
//    return db;
//}


TEST_F(BitcoinLikeWalletSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    {
        auto nextIndex = wait(wallet->getNextAccountIndex());
        EXPECT_EQ(nextIndex, 0);
        auto account = createBitcoinLikeAccount(wallet, nextIndex, P2PKH_MEDIUM_XPUB_INFO);
        account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),
              make_receiver([=](const std::shared_ptr<api::Event> &event) {
                  fmt::print("Received event {}\n", api::to_string(event->getCode()));
                  if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                      return;
                  EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                  EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                  dispatcher->stop();
              }));
        dispatcher->waitUntilStopped();
    }
//    auto dispatcher = std::make_shared<QtThreadDispatcher>();
//    auto resolver = std::make_shared<NativePathResolver>();
//    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
//    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
//    auto http = std::make_shared<QtHttpClient>(dispatcher->getMainExecutionContext());
//    auto newPool = [=]() -> std::shared_ptr<WalletPool> {
//        return WalletPool::newInstance(
//                "my_pool",
//                Option<std::string>::NONE,
//                http,
//                nullptr,
//                resolver,
//                printer,
//                dispatcher,
//                nullptr,
//                backend,
//                api::DynamicObject::newInstance()
//        );
//    };
//    {
//        auto pool = newPool();
//        auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
//        auto nextIndex = wait(wallet->getNextAccountIndex());
//        EXPECT_EQ(nextIndex, 0);
//        auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(std::dynamic_pointer_cast<BitcoinLikeWallet>(wallet->asBitcoinLikeWallet())->createNewAccount(nextIndex, XPUB_PROVIDER)));
//
//        account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), make_receiver([=] (const std::shared_ptr<api::Event>& event) {
//            fmt::print("Received event {}\n", api::to_string(event->getCode()));
//            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
//                return ;
//            EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
//            if (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT) {
//
//            }
//            dispatcher->stop();
//        }));
//
//        dispatcher->waitUntilStopped();
//    }
//    resolver->clean();
}