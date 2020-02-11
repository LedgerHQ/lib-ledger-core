/*
 *
 * horizon_explorer_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 02/07/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include "StellarFixture.hpp"
#include <wallet/stellar/explorers/HorizonBlockchainExplorer.hpp>
#include <collections/DynamicObject.hpp>
#include <utils/DateUtils.hpp>

static const auto BASE_URL = "https://horizon-testnet.stellar.org";
static const auto MAINNET_URL = "https://horizon.stellar.org";


TEST_F(StellarFixture, GetAsset) {
    auto pool = newPool();
    auto explorer = std::make_shared<HorizonBlockchainExplorer>(
            pool->getDispatcher()->getSerialExecutionContext("explorer"),
            pool->getHttpClient(BASE_URL),
            std::make_shared<DynamicObject>()
            );
    auto asset = wait(explorer->getAsset("YY2", "GDDZFMRDTCIN5FS77SA6MHM5YUC6ZNKHFONPBSPZPIAGTXH7SMBGY4CH")).getValue();
    EXPECT_EQ(asset->type, "credit_alphanum4");
    EXPECT_EQ(asset->flags.authImmutable, false);
    EXPECT_EQ(asset->flags.authRequired, false);
    EXPECT_EQ(asset->flags.authRevocable, false);
    EXPECT_EQ(asset->code, "YY2");
    EXPECT_EQ(asset->issuer, "GDDZFMRDTCIN5FS77SA6MHM5YUC6ZNKHFONPBSPZPIAGTXH7SMBGY4CH");
}

TEST_F(StellarFixture, GetAccount) {
    auto pool = newPool();
    auto explorer = std::make_shared<HorizonBlockchainExplorer>(
            pool->getDispatcher()->getSerialExecutionContext("explorer"),
            pool->getHttpClient(MAINNET_URL),
            std::make_shared<DynamicObject>()
    );
    auto accountId = "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3";
    auto account = wait(explorer->getAccount(accountId));
    EXPECT_EQ(account->accountId, accountId);
    EXPECT_TRUE(!account->sequence.empty());
    EXPECT_TRUE(!account->flags.authImmutable);
    EXPECT_TRUE(!account->flags.authRevocable);
    EXPECT_TRUE(!account->flags.authRequired);
    bool foundBalance = false;
    for (const auto& balance : account->balances) {
        if (balance.assetType == "native") {
            EXPECT_TRUE(balance.value > BigInt::ZERO);
            foundBalance = true;
        }
    }
    EXPECT_TRUE(foundBalance);
}


TEST_F(StellarFixture, GetLastLedger) {
    auto pool = newPool();
    auto explorer = std::make_shared<HorizonBlockchainExplorer>(
            pool->getDispatcher()->getSerialExecutionContext("explorer"),
            pool->getHttpClient(BASE_URL),
            std::make_shared<DynamicObject>()
    );
    auto ledger = wait(explorer->getLastLedger());
    auto t = DateUtils::toJSON(ledger->time);
    EXPECT_TRUE(!ledger->hash.empty());
    EXPECT_TRUE(ledger->height > 69859L);
    EXPECT_TRUE(ledger->time > DateUtils::fromJSON("2015-07-16T23:49:00Z"));
}


TEST_F(StellarFixture, GetTransactions) {
    auto pool = newPool();
    auto explorer = std::make_shared<HorizonBlockchainExplorer>(
            pool->getDispatcher()->getSerialExecutionContext("explorer"),
            pool->getHttpClient(MAINNET_URL),
            std::make_shared<DynamicObject>()
    );
    auto accountId = "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3";
    auto transactions = wait(explorer->getTransactions(accountId, Option<std::string>::NONE));
    EXPECT_TRUE(transactions.size() >= 5);
    const auto& tx = transactions.front();
    EXPECT_EQ(tx->hash, "93645afbd9f1c60f364e5acf77acd542883549262e84fd813f7cfefd4dc5bad6");
    EXPECT_EQ(tx->successful, true);
    EXPECT_EQ(tx->ledger, 22921932UL);
    EXPECT_EQ(tx->createdAt, DateUtils::fromJSON("2019-03-14T10:08:27Z"));
    EXPECT_EQ(tx->sourceAccount, "GBV4ZDEPNQ2FKSPKGJP2YKDAIZWQ2XKRQD4V4ACH3TCTFY6KPY3OAVS7");
    EXPECT_EQ(tx->sourceAccountSequence, BigInt::fromString("48320671297607800"));
    EXPECT_EQ(tx->feePaid, BigInt(100));
    EXPECT_EQ(tx->memoType, "none");
    EXPECT_EQ(tx->memo, "");
}