/*
 *
 * query_builder_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/07/2017.
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

#include "BaseFixture.h"

#include <CoutLogPrinter.hpp>
#include <NativePathResolver.hpp>
#include <UvThreadDispatcher.hpp>
#include <api/QueryFilter.hpp>
#include <database/query/QueryBuilder.h>
#include <gtest/gtest.h>
#include <src/api/DynamicObject.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <src/wallet/pool/WalletPool.hpp>
#include <unordered_set>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>

class QueryBuilderTest : public BaseFixture {
};

TEST_F(QueryBuilderTest, SimpleOperationQuery) {
    auto pool = newDefaultPool();
    {
        const auto walletName = "my_wallet";
        auto wallet           = uv::wait(pool->createWallet(walletName, "bitcoin", api::DynamicObject::newInstance()));
        auto nextIndex        = uv::wait(wallet->getNextAccountIndex());
        EXPECT_EQ(nextIndex, 0);
        auto account                                                       = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
        std::vector<BitcoinLikeBlockchainExplorerTransaction> transactions = {
            *JSONUtils::parse<TransactionParser>(TX_1),
            *JSONUtils::parse<TransactionParser>(TX_2),
            *JSONUtils::parse<TransactionParser>(TX_3),
            *JSONUtils::parse<TransactionParser>(TX_4)};
        std::vector<ledger::core::Operation> ops;
        for (auto &tx : transactions) {
            account->interpretTransaction(tx, ops);
        }
        account->bulkInsert(ops);
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        soci::rowset<soci::row> rows = QueryBuilder()
                                           .select("uid")
                                           .from("operations")
                                           .to("o")
                                           .where(api::QueryFilter::operationUidEq("c099e0118230697140916ff925e4cd2dd9acdce97bbb80b31cab401a5b169ed1"))
                                           .execute(sql);
        auto count = 0;
        for (auto &row : rows) {
            EXPECT_EQ(row.get<std::string>(0), "c099e0118230697140916ff925e4cd2dd9acdce97bbb80b31cab401a5b169ed1");
            count += 1;
        }
        EXPECT_EQ(count, 1);
    }
    resolver->clean();
}
