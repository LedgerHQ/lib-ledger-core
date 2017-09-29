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
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto account = std::dynamic_pointer_cast<AbstractAccount>(wait(wallet->newAccountWithInfo(P2PKH_MEDIUM_KEYS_INFO)));
    auto address = wait(account->getFreshPublicAddresses())[0];
    EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
}