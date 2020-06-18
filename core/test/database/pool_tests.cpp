/*
 *
 * pool_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/04/2017.
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

using namespace ledger::core;
using namespace ledger::qt;

static const std::unordered_set<std::string> ALL_TABLE_NAMES = {
    "__database_meta__",
    "pools",
    "currencies",
    "units",
    "wallets",
    "accounts",
    "operations",
    "blocks",
    "bitcoin_currencies",
    "bitcoin_transactions",
    "bitcoin_inputs",
    "bitcoin_outputs",
    "bitcoin_operations",
    "bitcoin_transaction_inputs",
    "bitcoin_accounts",
    "bitcoin_operations"
};

TEST(DatabaseSessionPool, OpenAndMigrateForTheFirstTime) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    DatabaseSessionPool::getSessionPool(dispatcher->getSerialExecutionContext("worker"), backend, resolver, nullptr, "test")
    .onComplete(dispatcher->getMainExecutionContext(), [&] (const TryPtr<DatabaseSessionPool>& result) {
        EXPECT_TRUE(result.isSuccess());
        if (result.isFailure()) {
            std::cerr << result.getFailure().getMessage() << std::endl;
        } else {
            auto tables = ALL_TABLE_NAMES;
            soci::session sql(result.getValue()->getPool());
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT name FROM sqlite_master WHERE type = 'table'");
            for (auto& row : rows) {
                auto name = row.get<std::string>("name");
                tables.erase(name);
            }
            EXPECT_TRUE(tables.empty());
        }
        dispatcher->stop();
    });
    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST(DatabaseSessionPool, InitializeCurrencies) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto pool = WalletPool::newInstance(
            "my_pool",
            "",
            nullptr,
            nullptr,
            resolver,
            printer,
            dispatcher,
            nullptr,
            backend,
            api::DynamicObject::newInstance(),
            nullptr,
            nullptr
    );

    api::Currency bitcoin;

    for (auto& currency : pool->getCurrencies()) {
        if (currency.name == "bitcoin") {
            bitcoin = currency;
        }
    }

    EXPECT_EQ(bitcoin.name, "bitcoin");
    EXPECT_EQ(bitcoin.bip44CoinType, 0);
    EXPECT_EQ(bitcoin.paymentUriScheme, "bitcoin");
    EXPECT_EQ(bitcoin.bitcoinLikeNetworkParameters.value().P2PKHVersion[0], 0);
    EXPECT_EQ(bitcoin.bitcoinLikeNetworkParameters.value().P2SHVersion[0], 5);

    for (const auto& unit : bitcoin.units) {
        if (unit.name == "bitcoin") {
            EXPECT_EQ(unit.code, "BTC");
            EXPECT_EQ(unit.symbol, "BTC");
            EXPECT_EQ(unit.numberOfDecimal, 8);
        } else if (unit.name == "milli-bitcoin") {
            EXPECT_EQ(unit.code, "mBTC");
            EXPECT_EQ(unit.symbol, "mBTC");
            EXPECT_EQ(unit.numberOfDecimal, 5);
        }
    }

    resolver->clean();
}
