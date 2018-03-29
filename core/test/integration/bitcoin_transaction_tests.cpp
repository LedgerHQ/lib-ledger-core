/*
 *
 * bitcoin_transaction_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/03/2018.
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
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>

struct BitcoinMakeTransaction : public BaseFixture {

    void SetUp() override {
        BaseFixture::SetUp();
        recreate();
    }

    void recreate() {
        pool = newDefaultPool();
        wallet = wait(pool->createWallet("my_wallet", "bitcoin", DynamicObject::newInstance()));
        p2pkh_account = ledger::testing::medium_xpub::inflate(pool, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        BaseFixture::TearDown();
        pool = nullptr;
        wallet = nullptr;
        p2pkh_account = nullptr;
    }

    std::shared_ptr<BitcoinLikeTransactionBuilder> p2pkh_tx_builder() {
        return std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(p2pkh_account->buildTransaction());
    }

    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<BitcoinLikeAccount> p2pkh_account;
    api::Currency currency;
};

TEST_F(BitcoinMakeTransaction, CreateStandardP2PKHWithOneOutput) {
    auto builder = p2pkh_tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 10000), "14GH47aGFWSjvdrEiYTEfwjgsphNtbkWzP");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
    auto tx = wait(builder->build());
}