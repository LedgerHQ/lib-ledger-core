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

class AccountCreationTest : public BaseFixture {};

TEST_F(AccountCreationTest, CreateBitcoinAccountWithInfo) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(uv::wait(wallet->newAccountWithInfo(P2PKH_MEDIUM_KEYS_INFO)));
    auto address = uv::wait(account->getFreshPublicAddresses())[0]->toString();
    EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
    uv::wait(pool->deleteWallet("my_wallet"));
}

TEST_F(AccountCreationTest, CreateBitcoinAccountWithInfoOnExistingWallet) {
    {
        auto pool = newDefaultPool();
        auto wallet = uv::wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    }
    {
        auto pool = newDefaultPool();
        EXPECT_THROW(uv::wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance())), Exception);
        auto wallet = uv::wait(pool->getWallet("my_wallet"));
        auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(uv::wait(wallet->newAccountWithInfo(P2PKH_MEDIUM_KEYS_INFO)));
        auto address = uv::wait(account->getFreshPublicAddresses())[0]->toString();
        EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
        uv::wait(pool->deleteWallet("my_wallet"));
    }
}

TEST_F(AccountCreationTest, ChangePassword) {
    auto oldPassword = "";
    auto newPassword = "new_test";

    // Create wallet, account ... in plain DB
    auto pool = newDefaultPool("my_pool", oldPassword);
    {
        auto wallet = uv::wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
        auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(uv::wait(wallet->newAccountWithInfo(P2PKH_MEDIUM_KEYS_INFO)));
        auto address = uv::wait(account->getFreshPublicAddresses())[0]->toString();
        EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
    }

    auto changePasswordAndGetInfos = [] (const std::shared_ptr<WalletPool> &walletPool, const std::string &oldPassword, const std::string &newPassword) {
        uv::wait(walletPool->changePassword(oldPassword, newPassword));
        {
            auto wallet = uv::wait(walletPool->getWallet("my_wallet"));
            auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(uv::wait(wallet->getAccount(0)));
            auto address = uv::wait(account->getFreshPublicAddresses())[0]->toString();
            EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
        }
    };

    changePasswordAndGetInfos(pool, "", "new_test");
    changePasswordAndGetInfos(pool, "new_test", "new_test_0");
    changePasswordAndGetInfos(pool, "new_test_0", "");
    uv::wait(pool->deleteWallet("my_wallet"));
}