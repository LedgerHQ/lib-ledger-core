/*
 *
 * bitcoin_like_account_info_test.cpp.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 06/09/2017.
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

class AccountInfoTests : public BaseFixture {

};

TEST_F(AccountInfoTests, FirstAccountInfo) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto info = wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/0'");
    EXPECT_EQ(info.owners[1], "main");
    EXPECT_EQ(info.derivations[1], "44'/0'/0'");
}

TEST_F(AccountInfoTests, FirstEthAccountInfo) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "ethereum", DynamicObject::newInstance()));
    auto info = wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/60'/0'");
    //api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>"
}

TEST_F(AccountInfoTests, FirstEthCustomDerivationAccountInfo) {
    auto config = DynamicObject::newInstance();
    config->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>");
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "ethereum", DynamicObject::newInstance()));
    auto info = wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/60'/0'");
}

TEST_F(AccountInfoTests, AnotherAccountInfo) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto info = wait(wallet->getAccountCreationInfo(20));
    EXPECT_EQ(info.index, 20);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/0'");
    EXPECT_EQ(info.owners[1], "main");
    EXPECT_EQ(info.derivations[1], "44'/0'/20'");
}