/*
 *
 * synchronization_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/07/2017.
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
#include "BaseFixture.h"

class BitcoinLikeWalletSynchronization : public BaseFixture {

};

TEST_F(BitcoinLikeWalletSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin",
                                              api::DynamicObject::newInstance()));
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2PKH_MEDIUM_XPUB_INFO);
            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),
                                           make_receiver([](const std::shared_ptr<api::Event> &event) {
                                               fmt::print("Received event {}\n", api::to_string(event->getCode()));
                                           }));
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),
                                              make_receiver([=](const std::shared_ptr<api::Event> &event) {
                                                  fmt::print("Received event {}\n", api::to_string(event->getCode()));
                                                  if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                                                      return;
                                                  EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                                                  EXPECT_EQ(event->getCode(),
                                                            api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                                                  dispatcher->stop();
                                              }));
            dispatcher->waitUntilStopped();
        }
    }
}

TEST_F(BitcoinLikeWalletSynchronization, SynchronizeOnceAtATime) {
    auto pool = newDefaultPool();
    {
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a62", "bitcoin",
                                              api::DynamicObject::newInstance()));
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2PKH_MEDIUM_XPUB_INFO);
            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),
                                           make_receiver([](const std::shared_ptr<api::Event> &event) {
                                               fmt::print("Received event {}\n", api::to_string(event->getCode()));
                                           }));
            auto bus = account->synchronize();
            bus->subscribe(dispatcher->getMainExecutionContext(),
                                              make_receiver([=](const std::shared_ptr<api::Event> &event) {
                                                  fmt::print("Received event {}\n", api::to_string(event->getCode()));
                                                  if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                                                      return;
                                                  EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                                                  EXPECT_EQ(event->getCode(),
                                                            api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                                                  dispatcher->stop();
                                              }));
            EXPECT_EQ(bus, account->synchronize());
            dispatcher->waitUntilStopped();
        }
    }
}

TEST_F(BitcoinLikeWalletSynchronization, SynchronizeFromLastBlock) {
    auto pool = newDefaultPool();
    {
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a63", "bitcoin",
                                              api::DynamicObject::newInstance()));
        createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
        auto synchronize = [wallet, pool, this] (bool expectNewOp) {
            auto account = wait(wallet->getAccount(0));
            auto numberOfOp = 0;
            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),
                                           make_receiver([&numberOfOp](const std::shared_ptr<api::Event> &event) {
                                                numberOfOp += 1;
                                           }));
            auto bus = account->synchronize();
            bus->subscribe(dispatcher->getMainExecutionContext(),
                           make_receiver([=, &numberOfOp](const std::shared_ptr<api::Event> &event) {
                               fmt::print("Received event {}\n", api::to_string(event->getCode()));
                               if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                                   return;
                               EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                               EXPECT_EQ(expectNewOp, (numberOfOp > 0));
                               dispatcher->stop();
                           }));
            EXPECT_EQ(bus, account->synchronize());
            dispatcher->waitUntilStopped();
            return bus;
        };
        auto b1 = synchronize(true);
        auto b2 = synchronize(false);
        EXPECT_NE(b1, b2);
    }
}