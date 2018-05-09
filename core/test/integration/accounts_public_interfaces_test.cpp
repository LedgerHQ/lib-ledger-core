/*
 *
 * accounts_public_interfaces_test.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/09/2017.
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

#include "BaseFixture.h"
#include "../fixtures/medium_xpub_fixtures.h"
#include "../fixtures/segwit_xpub_fixtures.h"
#include <wallet/common/OperationQuery.h>
#include <api/KeychainEngines.hpp>
#include <iostream>
using namespace std;
class AccountsPublicInterfaceTest : public BaseFixture {
public:
    void SetUp() override {
        BaseFixture::SetUp();
        recreate();
    }

    void recreate() {
        pool = newDefaultPool();
        wallet = wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    }

    void TearDown() override {
        BaseFixture::TearDown();
        pool = nullptr;
        wallet = nullptr;
    }

    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<AbstractWallet> wallet;
};

TEST_F(AccountsPublicInterfaceTest, GetAddressOnEmptyAccount) {
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto addresses = wait(account->getFreshPublicAddresses());
    EXPECT_EQ(addresses.size(), 20);
    EXPECT_EQ(addresses.front()->toString(), "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
}

TEST_F(AccountsPublicInterfaceTest, GetBalanceOnEmptyAccount) {
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto balance = wait(account->getBalance());
    EXPECT_EQ(balance->toMagnitude(0)->toLong(), 0);
}

TEST_F(AccountsPublicInterfaceTest, GetBalanceOnAccountWithSomeTxs) {
    auto account = ledger::testing::medium_xpub::inflate(pool, wallet);
    auto balance = wait(account->getBalance());
    auto utxos = wait(account->getUTXO());
    auto uxtoCount = wait(account->getUTXOCount());
    EXPECT_EQ(balance->toLong(), 143590816L);
    EXPECT_EQ(utxos.size(), 8);
    EXPECT_EQ(uxtoCount, 8);
}

TEST_F(AccountsPublicInterfaceTest, GetBalanceHistoryOnAccountWithSomeTxs) {
    auto account = ledger::testing::medium_xpub::inflate(pool, wallet);
    auto fromDate = "2017-10-12T13:38:23Z";
    auto toDate = "2018-10-12T13:38:23Z";
    auto balanceHistory = wait(account->getBalanceHistory(fromDate, toDate, api::TimePeriod::WEEK));
    EXPECT_EQ(balanceHistory.size(), 52);
    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), 91890664L);
}

TEST_F(AccountsPublicInterfaceTest, QueryOperations) {
    auto account = ledger::testing::medium_xpub::inflate(pool, wallet);
    auto query = std::dynamic_pointer_cast<ledger::core::OperationQuery>(account->queryOperations()->limit(100)->partial());
    auto operations = wait(query->execute());
    EXPECT_EQ(operations.size(), 100);
}

TEST_F(AccountsPublicInterfaceTest, QueryOperationsOnEmptyAccount) {
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto query = std::dynamic_pointer_cast<ledger::core::OperationQuery>(account->queryOperations()->limit(100)->partial());
    auto operations = wait(query->execute());
    EXPECT_EQ(operations.size(), 0);
}

TEST_F(AccountsPublicInterfaceTest, GetTestnetUnits) {
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
    wallet = wait(pool->createWallet("my_wallet_testnet", "bitcoin_testnet", configuration));
    auto currency = pool->getCurrency("bitcoin");
    EXPECT_EQ(currency->name, "bitcoin");
    cout<<">>>> Get account"<<endl;
    auto account = ledger::testing::segwit_xpub::inflate(pool, wallet);
    cout<<">>>> Get balance"<<endl;
    auto balance = wait(account->getBalance());
    cout<<">>>> Get balance toLong"<<endl;
    auto balanceLong = balance->toLong();
    //EXPECT_EQ(balanceLong, 100L);
    //auto balance = wait(account->getBalance());
    //auto balance =
    //EXPECT_EQ();
}
