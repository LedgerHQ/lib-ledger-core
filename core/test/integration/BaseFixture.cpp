/*
 *
 * BaseFixture.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include <utils/FilesystemUtils.h>
#include "BaseFixture.h"

api::ExtendedKeyAccountCreationInfo P2PKH_MEDIUM_XPUB_INFO(
        0, {"main"}, {"44'/0'/0'"}, {"xpub6D4waFVPfPCpRvPkQd9A6n65z3hTp6TvkjnBHG5j2MCKytMuadKgfTUHqwRH77GQqCKTTsUXSZzGYxMGpWpJBdYAYVH75x7yMnwJvra1BUJ"}
);

api::ExtendedKeyAccountCreationInfo P2PKH_BIG_XPUB_INFO(
        0, {"main"}, {"44'/0'/0'"}, {"xpub6CThYZbX4PTeA7KRYZ8YXP3F6HwT2eVKPQap3Avieds3p1eos35UzSsJtTbJ3vQ8d3fjRwk4bCEz4m4H6mkFW49q29ZZ6gS8tvahs4WCZ9X"}
);

void BaseFixture::SetUp() {
    ::testing::Test::SetUp();
    ledger::qt::FilesystemUtils::clearFs();
    dispatcher = std::make_shared<QtThreadDispatcher>();
    resolver = std::make_shared<NativePathResolver>();
    backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    http = std::make_shared<QtHttpClient>(dispatcher->getMainExecutionContext());
}

void BaseFixture::TearDown() {
    ::testing::Test::TearDown();
    resolver->clean();
}

std::shared_ptr<WalletPool> BaseFixture::newDefaultPool(std::string poolName) {
    return WalletPool::newInstance(
            poolName,
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
}

BitcoinLikeWalletDatabase
BaseFixture::newBitcoinAccount(const std::shared_ptr<WalletPool> &pool, const std::string &walletName, int32_t index,
                        const std::string &xpub) {
    BitcoinLikeWalletDatabase db(pool, walletName, "bitcoin");
    if (!db.accountExists(index)) {
        createWallet(pool, walletName);
        createAccount(pool, walletName, index);
        db.createAccount(index, xpub);
    }
    return db;
}

void BaseFixture::createWallet(const std::shared_ptr<WalletPool> &pool, const std::string &walletName) {
    soci::session sql(pool->getDatabaseSessionPool()
                              ->getPool());
    WalletDatabaseEntry entry;
    entry.configuration = std::static_pointer_cast<DynamicObject>(DynamicObject::newInstance());
    entry.name = walletName;
    entry.poolName = pool->getName();
    entry.currencyName = "bitcoin";
    entry.updateUid();
    PoolDatabaseHelper::putWallet(sql, entry);
}

void BaseFixture::createAccount(const std::shared_ptr<WalletPool> &pool, const std::string &walletName, int32_t index) {
    soci::session sql(pool->getDatabaseSessionPool()
                              ->getPool());
    auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), walletName, "bitcoin");
    if (!AccountDatabaseHelper::accountExists(sql, walletUid, index))
        AccountDatabaseHelper::createAccount(sql, walletUid, index);
}

std::shared_ptr<BitcoinLikeAccount>
BaseFixture::createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<BitcoinLikeAccount>
BaseFixture::createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::ExtendedKeyAccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->newAccountWithExtendedKeyInfo(info)));
}

