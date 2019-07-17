/*
 *
 * synchronization_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 01/04/2019.
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
#include <wallet/common/OperationQuery.h>
#include <math/BigInt.h>

TEST_F(StellarFixture, SynchronizeStellarAccount) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet", "stellar", api::DynamicObject::newInstance());
    auto info = ::wait(wallet->getNextAccountCreationInfo());
    auto account = newAccount(wallet, 0, defaultAccount());

    auto bus = account->synchronize();
    bus->subscribe(dispatcher->getMainExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       dispatcher->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    dispatcher->waitUntilStopped();
    auto balance = ::wait(account->getBalance());
    auto operations = ::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations())->execute());
    EXPECT_TRUE(balance->toBigInt()->compare(api::BigInt::fromLong(0)) > 0);
    EXPECT_TRUE(operations.size() >= 5);
}

