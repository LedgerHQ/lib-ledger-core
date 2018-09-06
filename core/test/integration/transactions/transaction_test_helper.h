/*
 *
 * transaction_test_helper
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 16/05/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#ifndef LEDGER_CORE_TRANSACTION_TEST_HELPER_H
#define LEDGER_CORE_TRANSACTION_TEST_HELPER_H

#include <string>
#include <api/DynamicObject.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include "../BaseFixture.h"


using namespace ledger::core;

struct TransactionTestData {
    std::shared_ptr<api::DynamicObject> configuration;
    std::string walletName;
    std::string currencyName;
    std::function<std::shared_ptr<BitcoinLikeAccount> (const std::shared_ptr<WalletPool>&,
                                                       const std::shared_ptr<AbstractWallet>& )> inflate;
};

struct BitcoinMakeBaseTransaction : public BaseFixture {

    void SetUp() override {
        BaseFixture::SetUp();
        SetUpConfig();
        recreate();
    }

    void recreate() {
        pool = newDefaultPool();
        wallet = wait(pool->createWallet(testData.walletName, testData.currencyName, testData.configuration));
        account = testData.inflate(pool, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        BaseFixture::TearDown();
        pool = nullptr;
        wallet = nullptr;
        account = nullptr;
    }

    std::shared_ptr<BitcoinLikeTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(account->buildTransaction());
    }
    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<BitcoinLikeAccount> account;
    api::Currency currency;
    TransactionTestData testData;

protected:
    virtual void SetUpConfig() = 0;
};



#endif //LEDGER_CORE_TRANSACTION_TEST_HELPER_H
