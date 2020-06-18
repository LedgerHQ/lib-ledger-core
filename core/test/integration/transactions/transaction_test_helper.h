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
#include <wallet/ethereum/EthereumLikeWallet.h>
#include <wallet/ethereum/EthereumLikeAccount.h>
#include <wallet/ethereum/transaction_builders/EthereumLikeTransactionBuilder.h>
#include <wallet/ripple/RippleLikeWallet.h>
#include <wallet/ripple/RippleLikeAccount.h>
#include <wallet/ripple/transaction_builders/RippleLikeTransactionBuilder.h>
#include <wallet/tezos/TezosLikeWallet.h>
#include <wallet/tezos/TezosLikeAccount.h>
#include <wallet/tezos/transaction_builders/TezosLikeTransactionBuilder.h>
#include "../BaseFixture.h"


using namespace ledger::core;

struct TransactionTestData {
    std::shared_ptr<api::DynamicObject> configuration;
    std::string walletName;
    std::string currencyName;
    std::function<std::shared_ptr<BitcoinLikeAccount> (const std::shared_ptr<WalletPool>&,
                                                       const std::shared_ptr<AbstractWallet>& )> inflate_btc;
    std::function<std::shared_ptr<EthereumLikeAccount> (const std::shared_ptr<WalletPool>&,
                                                       const std::shared_ptr<AbstractWallet>& )> inflate_eth;

    std::function<std::shared_ptr<RippleLikeAccount> (const std::shared_ptr<WalletPool>&,
                                                      const std::shared_ptr<AbstractWallet>& )> inflate_xrp;

    std::function<std::shared_ptr<TezosLikeAccount> (const std::shared_ptr<WalletPool>&,
                                                     const std::shared_ptr<AbstractWallet>& )> inflate_xtz;
};

struct BitcoinMakeBaseTransaction : public BaseFixture {

    void SetUp() override {
        BaseFixture::SetUp();
        SetUpConfig();
        recreate();
    }

    virtual void recreate() {
        pool = newDefaultPool();
        wallet = wait(pool->createWallet(testData.walletName, testData.currencyName, testData.configuration));
        account = testData.inflate_btc(pool, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        BaseFixture::TearDown();
        pool.reset();
        wallet.reset();
        account.reset();
    }

    std::shared_ptr<BitcoinLikeTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(account->buildTransaction(false));
    }
    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<BitcoinLikeAccount> account;
    api::Currency currency;
    TransactionTestData testData;

protected:
    virtual void SetUpConfig() = 0;
};

struct EthereumMakeBaseTransaction : public BaseFixture {

    void SetUp() override {
        BaseFixture::SetUp();
        SetUpConfig();
        recreate();
    }

    void recreate() {
        pool = newDefaultPool();
        wallet = wait(pool->createWallet(testData.walletName, testData.currencyName, testData.configuration));
        account = testData.inflate_eth(pool, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        BaseFixture::TearDown();
        pool.reset();
        wallet.reset();
        account.reset();
    }

    std::shared_ptr<EthereumLikeTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<EthereumLikeTransactionBuilder>(account->buildTransaction());
    }
    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<EthereumLikeAccount> account;
    api::Currency currency;
    TransactionTestData testData;

protected:
    virtual void SetUpConfig() = 0;
};

struct RippleMakeBaseTransaction : public BaseFixture {

    void SetUp() override {
        BaseFixture::SetUp();
        SetUpConfig();
        recreate();
    }

    void recreate() {
        pool = newDefaultPool();
        wallet = wait(pool->createWallet(testData.walletName, testData.currencyName, testData.configuration));
        account = testData.inflate_xrp(pool, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        BaseFixture::TearDown();
        pool.reset();
        wallet.reset();
        account.reset();
    }

    std::shared_ptr<RippleLikeTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<RippleLikeTransactionBuilder>(account->buildTransaction());
    }
    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<RippleLikeAccount> account;
    api::Currency currency;
    TransactionTestData testData;

protected:
    virtual void SetUpConfig() = 0;
};

struct TezosMakeBaseTransaction : public BaseFixture {

    void SetUp() override {
        BaseFixture::SetUp();
        SetUpConfig();
        recreate();
    }

    void recreate() {
        pool = newDefaultPool();
        wallet = wait(pool->createWallet(testData.walletName, testData.currencyName, testData.configuration));
        account = testData.inflate_xtz(pool, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        BaseFixture::TearDown();
        pool.reset();
        wallet.reset();
        account.reset();
    }

    std::shared_ptr<TezosLikeTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<TezosLikeTransactionBuilder>(account->buildTransaction());
    }
    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<TezosLikeAccount> account;
    api::Currency currency;
    TransactionTestData testData;

protected:
    virtual void SetUpConfig() = 0;
};



#endif //LEDGER_CORE_TRANSACTION_TEST_HELPER_H
