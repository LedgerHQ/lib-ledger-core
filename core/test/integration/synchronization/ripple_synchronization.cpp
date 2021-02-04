/*
 *
 * ripple_synchronization
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#include <gtest/gtest.h>
#include "../BaseFixture.h"
#include <set>
#include <api/KeychainEngines.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/ripple/database/RippleLikeAccountDatabaseHelper.h>
#include <wallet/ripple/transaction_builders/RippleLikeTransactionBuilder.h>
#include <iostream>
#include <api/BlockchainExplorerEngines.hpp>
#include <api/RippleLikeOperation.hpp>
#include <api/RippleLikeTransaction.hpp>

using namespace std;

class RippleLikeWalletSynchronization : public BaseFixture {

};

TEST_F(RippleLikeWalletSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = uv::wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ripple", configuration));
        {
            auto nextIndex = uv::wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createRippleLikeAccount(wallet, nextIndex, XRP_KEYS_INFO);

            auto fees = uv::wait(account->getFees());
            EXPECT_GT(fees->toLong(), 0L);
            auto baseReserve = uv::wait(account->getBaseReserve());
            EXPECT_GT(baseReserve->toLong(), 0L);

            auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = uv::wait(account->getBalance());
                std::cout << "Balance: " << balance->toString() << std::endl;
                auto txBuilder = std::dynamic_pointer_cast<RippleLikeTransactionBuilder>(account->buildTransaction());
                getTestExecutionContext()->stop();
            });

            auto restoreKey = account->getRestoreKey();
            auto bus = account->synchronize();
            bus->subscribe(getTestExecutionContext(), receiver);

            getTestExecutionContext()->waitUntilStopped();

            auto ops = uv::wait(
                    std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete()
                    ->addOrder(api::OperationOrderKey::DATE,false))->execute());
            std::cout << "Ops: " << ops.size() << std::endl;

            auto firstOp = ops.front();
            auto firstXrpOp = firstOp->asRippleLikeOperation();

            EXPECT_EQ(firstOp->getSenders()[0], "rPVMhWBsfF9iMXYj3aAzJVkPDTFNSyWdKy");
            EXPECT_EQ(firstOp->getRecipients()[0], "rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7");
            EXPECT_EQ(firstOp->getBlockHeight().value(), 48602088);
            EXPECT_EQ(firstOp->getAmount()->toLong(), 49000000);
            EXPECT_EQ(firstOp->getFees()->toLong(), 6235);
            EXPECT_EQ(firstXrpOp->getTransaction()->getDestinationTag().value(), 0);
            EXPECT_EQ(firstXrpOp->getTransaction()->getLedgerSequence()->intValue(), 48602088);
            EXPECT_EQ(firstXrpOp->getTransaction()->getDate(), DateUtils::fromJSON("2019-07-12T11:05:20Z"));

            for (auto const& op : ops) {
                auto xrpOp = op->asRippleLikeOperation();
                EXPECT_FALSE(xrpOp == nullptr);
                EXPECT_FALSE(xrpOp->getTransaction()->getSequence() == nullptr);
                EXPECT_TRUE(std::chrono::duration_cast<std::chrono::hours>(xrpOp->getTransaction()->getDate().time_since_epoch()).count() != 0);
            }

            EXPECT_EQ(uv::wait(account->isAddressActivated("rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7")), true);
            EXPECT_EQ(uv::wait(account->isAddressActivated("rageXHB6Q4VbvvWdTzKANwjeCT4HXFCK")), false);
            EXPECT_EQ(uv::wait(account->isAddressActivated("rf1pjatD8LyyevP1BqQJtHoz5edC5vE77Q")), false);

            auto block = uv::wait(account->getLastBlock());
            EXPECT_GT(block.height, 0);
            EXPECT_LT(block.height, (std::numeric_limits<int32_t>::max)());
        }
    }
}

TEST_F(RippleLikeWalletSynchronization, BalanceHistory) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = uv::wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ripple", configuration));
        {
            auto nextIndex = uv::wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createRippleLikeAccount(wallet, nextIndex, XRP_KEYS_INFO);

            std::shared_ptr<Amount> balance;

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                balance = uv::wait(account->getBalance());
                std::cout << "Balance: " << balance->toString() << std::endl;
                auto txBuilder = std::dynamic_pointer_cast<RippleLikeTransactionBuilder>(account->buildTransaction());
                getTestExecutionContext()->stop();
            });

            auto restoreKey = account->getRestoreKey();
            auto bus = account->synchronize();
            bus->subscribe(getTestExecutionContext(), receiver);

            getTestExecutionContext()->waitUntilStopped();

            auto now = std::time(nullptr);
            char now_str[256];
            std::strftime(now_str, sizeof(now_str), "%Y-%m-%dT%H:%M:%SZ", std::localtime(&now));

            auto history = uv::wait(account->getBalanceHistory(
                "2019-09-20T00:00:00Z",
                now_str,
                api::TimePeriod::DAY
            ));

            EXPECT_EQ(history.back()->toString(), balance->toString());

            auto zero = std::make_shared<api::BigIntImpl>(BigInt::ZERO);
            for (auto const& balance : history) {
                EXPECT_TRUE(balance->toBigInt()->compare(zero) > 0);
            }
        }
    }
}

TEST_F(RippleLikeWalletSynchronization, VaultAccountSynchronization) {
    auto pool = newDefaultPool();
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                             "44'/<coin_type>'/<account>'/<node>/<address>");
    auto wallet = uv::wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ripple", configuration));
    auto nextIndex = uv::wait(wallet->getNextAccountIndex());
    auto account = createRippleLikeAccount(wallet, nextIndex, VAULT_XRP_KEYS_INFO);
    auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED);

        dispatcher->stop();
    });

    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);
    dispatcher->waitUntilStopped();

    auto ops = uv::wait(
            std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    std::cout << "Ops: " << ops.size() << std::endl;

    uint32_t destinationTag = 0;
    for (auto const& op : ops) {
        auto xrpOp = op->asRippleLikeOperation();

        if (xrpOp->getTransaction()->getHash() == "EE38840B83CAB39216611D2F6E4F9828818514C3EA47504AE2521D8957331D3C" ) {
          destinationTag = xrpOp->getTransaction()->getDestinationTag().value_or(0);
          break;
        }
    }

    EXPECT_TRUE(destinationTag == 123456789);
}
