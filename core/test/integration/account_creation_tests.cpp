/*
 *
 * account_creation_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/09/2017.
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
#include <Uuid.hpp>

class AccountCreationTest : public BaseFixture {};

TEST_F(AccountCreationTest, CreateBitcoinAccountWithInfo) {
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->newAccountWithInfo(P2PKH_MEDIUM_KEYS_INFO)));
    auto address = wait(account->getFreshPublicAddresses())[0]->toString();
    EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
}

TEST_F(AccountCreationTest, CreateBitcoinAccountWithInfoOnExistingWallet) {
    auto poolName = uuid::generate_uuid_v4();
    auto walletName = uuid::generate_uuid_v4();
    {
        auto pool = newDefaultPool(poolName);
        auto wallet = wait(pool->createWallet(walletName, "bitcoin", DynamicObject::newInstance()));
    }
    {
        auto pool = newDefaultPool(poolName);
        EXPECT_THROW(wait(pool->createWallet(walletName, "bitcoin", DynamicObject::newInstance())), Exception);
        auto wallet = wait(pool->getWallet(walletName));
        auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->newAccountWithInfo(P2PKH_MEDIUM_KEYS_INFO)));
        auto address = wait(account->getFreshPublicAddresses())[0]->toString();
        EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
    }
}

TEST_F(AccountCreationTest, ChangePassword) {
    auto oldPassword = "";
    auto newPassword = "new_test";

    // Create wallet, account ... in plain DB
    auto pool = newDefaultPool(uuid::generate_uuid_v4(), oldPassword);
    {
        auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
        auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->newAccountWithInfo(P2PKH_MEDIUM_KEYS_INFO)));
        auto address = wait(account->getFreshPublicAddresses())[0]->toString();
        EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
    }

    auto changePasswordAndGetInfos = [] (const std::shared_ptr<WalletPool> &walletPool, const std::string &oldPassword, const std::string &newPassword) {
        wait(walletPool->changePassword(oldPassword, newPassword));
        {
            auto wallet = wait(walletPool->getWallet("my_wallet"));
            auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->getAccount(0)));
            auto address = wait(account->getFreshPublicAddresses())[0]->toString();
            EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
        }
    };

    changePasswordAndGetInfos(pool, "", "new_test");
    changePasswordAndGetInfos(pool, "new_test", "new_test_0");
    changePasswordAndGetInfos(pool, "new_test_0", "");
}
