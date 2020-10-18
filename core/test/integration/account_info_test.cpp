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
    auto wallet = uv::wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/0'");
    EXPECT_EQ(info.owners[1], "main");
    EXPECT_EQ(info.derivations[1], "44'/0'/0'");
    uv::wait(pool->deleteWallet("my_wallet"));
}

TEST_F(AccountInfoTests, FirstEthAccountInfo) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet("my_wallet", "ethereum", DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/60'/0'");
    //api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>"
    uv::wait(pool->deleteWallet("my_wallet"));
}

TEST_F(AccountInfoTests, FirstXRPAccountInfo) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet("my_wallet", "ripple", DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners.size(), 1);
    EXPECT_EQ(info.derivations.size(), 1);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/144'/0'");
    uv::wait(pool->deleteWallet("my_wallet"));
}

TEST_F(AccountInfoTests, FirstXTZAccountInfo) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet("my_wallet", "tezos", DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners.size(), 1);
    EXPECT_EQ(info.derivations.size(), 1);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/1729'/0'");
    uv::wait(pool->deleteWallet("my_wallet"));
}

TEST_F(AccountInfoTests, FirstEthCustomDerivationAccountInfo) {
    auto config = DynamicObject::newInstance();
    config->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>");
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet("my_wallet", "ethereum", DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/60'/0'");
    uv::wait(pool->deleteWallet("my_wallet"));
}

TEST_F(AccountInfoTests, AnotherAccountInfo) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getAccountCreationInfo(20));
    EXPECT_EQ(info.index, 20);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/0'");
    EXPECT_EQ(info.owners[1], "main");
    EXPECT_EQ(info.derivations[1], "44'/0'/20'");
    uv::wait(pool->deleteWallet("my_wallet"));
}

TEST_F(AccountInfoTests, GetAddressFromRange) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);

    auto freshAddresses = uv::wait(account->getFreshPublicAddresses());

    auto from = 10, to = 100;
    auto addresses = uv::wait(account->getAddresses(from, to));

    EXPECT_EQ(addresses.size(), 2 * (to - from + 1));
    
    // Observable range gives us 20 so it should be fine
    if (freshAddresses.size() > 12) {
        EXPECT_EQ(addresses[0]->toString(), freshAddresses[10]->toString());
        EXPECT_EQ(addresses[2]->toString(), freshAddresses[11]->toString());
        EXPECT_EQ(addresses[4]->toString(), freshAddresses[12]->toString());
    }

    EXPECT_EQ(addresses[2 * (to - from + 1) - 1]->toString(), "1167QbGjTWVK3etniJwua6wybBKkS7Lr8w");
    uv::wait(pool->deleteWallet("my_wallet"));
}