/*
 *
 * create_stellar_account_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/02/2019.
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

TEST_F(StellarFixture, CreateAccountWithPubKey) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet", "stellar", api::DynamicObject::newInstance());
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto a = newAccount(wallet, 0, defaultAccount());
    auto account = std::static_pointer_cast<AbstractAccount>(uv::wait(wallet->getAccount(0)));
    auto address = uv::wait(account->getFreshPublicAddresses()).front()->toString();
    auto derivation = uv::wait(account->getFreshPublicAddresses()).front()->getDerivationPath().value();
    EXPECT_EQ(address, "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(info.derivations.size(), 1);
    EXPECT_EQ(info.derivations[0], "44'/148'/0'");
    EXPECT_EQ(derivation, "");
}