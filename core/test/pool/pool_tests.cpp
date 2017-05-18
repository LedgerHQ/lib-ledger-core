/*
 *
 * pool_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/04/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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

#include <gtest/gtest.h>
#include <async/QtThreadDispatcher.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <NativePathResolver.hpp>
#include <unordered_set>
#include <src/wallet/pool/WalletPool.hpp>
#include <CoutLogPrinter.hpp>
#include <src/api/DynamicObject.hpp>
#include <wallet/common/CurrencyBuilder.hpp>

using namespace ledger::core;
using namespace ledger::qt;

TEST(WalletPool, InitializeCurrencies) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto pool = WalletPool::newInstance(
    "my_pool",
    Option<std::string>::NONE,
    nullptr,
    nullptr,
    resolver,
    printer,
    dispatcher,
    nullptr,
    backend,
    api::DynamicObject::newInstance()
    );

    api::Currency bitcoin;

    for (auto& currency : pool->getCurrencies()) {
        if (currency.name == "bitcoin") {
            bitcoin = currency;
        }
    }

    EXPECT_EQ(bitcoin.name, "bitcoin");
    EXPECT_EQ(bitcoin.bip44CoinType, 0);
    EXPECT_EQ(bitcoin.paymentUriScheme, "bitcoin");
    EXPECT_EQ(bitcoin.bitcoinLikeNetworkParameters.value().P2PKHVersion[0], 0);
    EXPECT_EQ(bitcoin.bitcoinLikeNetworkParameters.value().P2SHVersion[0], 5);

    for (const auto& unit : bitcoin.units) {
        if (unit.name == "bitcoin") {
            EXPECT_EQ(unit.code, "BTC");
            EXPECT_EQ(unit.symbol, "BTC");
            EXPECT_EQ(unit.numberOfDecimal, 8);
        } else if (unit.name == "milli-bitcoin") {
            EXPECT_EQ(unit.code, "mBTC");
            EXPECT_EQ(unit.symbol, "mBTC");
            EXPECT_EQ(unit.numberOfDecimal, 5);
        }
    }

    resolver->clean();
}

TEST(WalletPool, AddCurrency) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
        "my_pool",
        Option<std::string>::NONE,
        nullptr,
        nullptr,
        resolver,
        printer,
        dispatcher,
        nullptr,
        backend,
        api::DynamicObject::newInstance()
        );
    };
    api::BitcoinLikeNetworkParameters params(
    "wonder_coin", {42}, {21}, {42, 42, 21, 21}, api::BitcoinLikeFeePolicy::PER_KBYTE, 0,
    "Wonder Coin Signed Message:\n", false
    );
    api::Currency wonderCoin = CurrencyBuilder("wonder_coin").forkOfBitcoin(params).bip44(42).unit("wonder coin",
                                                                                                   12, "WC");
    {
        auto firstPool = newPool();
        bool found = false;
        bool foundInSecond = false;
        firstPool->addCurrency(wonderCoin).onComplete(dispatcher->getMainExecutionContext(),
                                                      [&](const Try<Unit> &unit) {
                                                          for (const auto &currency : firstPool->getCurrencies()) {
                                                              if (currency.name == "wonder_coin") {
                                                                  found = true;
                                                                  EXPECT_EQ(
                                                                  currency.bitcoinLikeNetworkParameters.value().MessagePrefix,
                                                                  wonderCoin.bitcoinLikeNetworkParameters.value().MessagePrefix);
                                                              }
                                                          }
                                                          dispatcher->stop();
                                                      });
        dispatcher->waitUntilStopped();
        EXPECT_TRUE(found);
    }
    {
        auto secondPool = newPool();
        bool found = false;
        for (const auto &currency : secondPool->getCurrencies()) {
            if (currency.name == "wonder_coin") {
                found = true;
                EXPECT_EQ(
                currency.bitcoinLikeNetworkParameters.value().MessagePrefix,
                wonderCoin.bitcoinLikeNetworkParameters.value().MessagePrefix);
            }
        }
        EXPECT_TRUE(found);
    }
   resolver->clean();
}

TEST(WalletPool, RemoveCurrency) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
        "my_pool",
        Option<std::string>::NONE,
        nullptr,
        nullptr,
        resolver,
        printer,
        dispatcher,
        nullptr,
        backend,
        api::DynamicObject::newInstance()
        );
    };
    api::BitcoinLikeNetworkParameters params(
    "wonder_coin", {42}, {21}, {42, 42, 21, 21}, api::BitcoinLikeFeePolicy::PER_KBYTE, 0,
    "Wonder Coin Signed Message:\n", false
    );
    api::Currency wonderCoin = CurrencyBuilder("wonder_coin").forkOfBitcoin(params).bip44(42).unit("wonder coin",
                                                                                                   12, "WC");
    {
        auto firstPool = newPool();
        bool found = false;
        bool foundInSecond = false;
        firstPool
        ->addCurrency(wonderCoin)
        .onComplete(dispatcher->getMainExecutionContext(),
          [&](const Try<Unit> &unit) {
              for (const auto &currency : firstPool->getCurrencies()) {
                  if (currency.name == "wonder_coin") {
                      found = true;
                      EXPECT_EQ(
                      currency.bitcoinLikeNetworkParameters.value().MessagePrefix,
                      wonderCoin.bitcoinLikeNetworkParameters.value().MessagePrefix);
                  }
              }
              firstPool->removeCurrency(wonderCoin.name).onComplete(dispatcher->getMainExecutionContext(),
                [&] (const Try<Unit>& result) {
                    dispatcher->stop();
                }
              );
          });
        dispatcher->waitUntilStopped();
        EXPECT_TRUE(found);
    }
    {
        auto secondPool = newPool();
        bool found = false;
        for (const auto &currency : secondPool->getCurrencies()) {
            if (currency.name == "wonder_coin") {
                found = true;
            }
        }
        EXPECT_FALSE(found);
    }
    resolver->clean();
}