/*
 *
 * synchronization_tests
 * ledger-core
 *
 * Created by Khalil Bellakrid on 15/05/2018.
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

#include <gtest/gtest.h>
#include "../BaseFixture.h"
#include <set>
#include <api/KeychainEngines.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/bitcoin/database/BitcoinLikeAccountDatabaseHelper.h>

class BitcoinLikeWalletP2SHSynchronization : public BaseFixture {

};

TEST_F(BitcoinLikeWalletP2SHSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin_testnet", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2SH_XPUB_INFO);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                dispatcher->stop();
            });

            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            dispatcher->waitUntilStopped();

            auto block = wait(account->getLastBlock());
            auto blockHash = block.blockHash;
        }
    }
}

TEST_F(BitcoinLikeWalletP2SHSynchronization, SynchronizeOnceAtATime) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a62", "bitcoin_testnet",configuration));
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2SH_XPUB_INFO);
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

TEST_F(BitcoinLikeWalletP2SHSynchronization, SynchronizeFromLastBlock) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a63", "bitcoin_testnet",configuration));
        createBitcoinLikeAccount(wallet, 0, P2SH_XPUB_INFO);
        auto synchronize = [wallet, pool, this] (bool expectNewOp) {
            auto account = wait(wallet->getAccount(0));
            auto numberOfOp = 0;

            auto receiverNumberOp = make_receiver([&numberOfOp](const std::shared_ptr<api::Event> &event) {
                numberOfOp += 1;
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiverNumberOp);
            auto bus = account->synchronize();

            auto receiver = make_receiver([=, &numberOfOp](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;

                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(expectNewOp, (numberOfOp > 0));
                dispatcher->stop();
            });

            bus->subscribe(dispatcher->getMainExecutionContext(),receiver);
            auto newBus = account->synchronize();
            EXPECT_EQ(bus, newBus);
            int res = dispatcher->waitUntilStopped();
            return bus;
        };

        auto b1 = synchronize(true);
        auto b2 = synchronize(false);
        EXPECT_NE(b1, b2);
    }
}

TEST_F(BitcoinLikeWalletP2SHSynchronization, EraseDataSinceAfterSynchronization) {
    auto pool = newDefaultPool();
    {
        //Set configuration
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "49'/<coin_type>'/<account>'/<node>/<address>");
        //Create wallet
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a63", "bitcoin_testnet", configuration));
        //Create account
        auto account = createBitcoinLikeAccount(wallet, 0, P2SH_XPUB_INFO);
        //Sync account
        auto bus = account->synchronize();
        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
            fmt::print("Received event {}\n", api::to_string(event->getCode()));
            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                return;

            EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
            dispatcher->stop();
        });
        bus->subscribe(dispatcher->getMainExecutionContext(),receiver);
        dispatcher->waitUntilStopped();

        auto accountCount = wait(wallet->getAccountCount());
        EXPECT_EQ(accountCount, 1);
        auto accountFromWallet = wait(wallet->getAccount(0));
        EXPECT_EQ(account, accountFromWallet);

        auto date = "2000-03-27T09:10:22Z";
        auto formatedDate = DateUtils::fromJSON(date);

        //Delete account
        auto code = wait(wallet->eraseDataSince(formatedDate));
        EXPECT_EQ(code, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        //Check if account was successfully deleted
        auto newAccountCount = wait(wallet->getAccountCount());
        EXPECT_EQ(newAccountCount, 0);
        {
            soci::session sql(pool->getDatabaseSessionPool()->getPool());
            BitcoinLikeAccountDatabaseEntry entry;
            auto result = BitcoinLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
            EXPECT_EQ(result, false);
        }

        //Delete wallet
        auto walletCode = wait(pool->eraseDataSince(formatedDate));
        EXPECT_EQ(walletCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        //Check if wallet was successfully deleted
        auto walletCount = wait(pool->getWalletCount());
        EXPECT_EQ(walletCount, 0);
    }
}
TEST_F(BitcoinLikeWalletP2SHSynchronization, TestNetSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = wait(pool->createWallet("testnet_wallet", "bitcoin_testnet",configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());
            info.extendedKeys.push_back("tpubDCcvqEHx7prGddpWTfEviiew5YLMrrKy4oJbt14teJZenSi6AYMAs2SNXwYXFzkrNYwECSmobwxESxMCrpfqw4gsUt88bcr8iMrJmbb8P2q");
            EXPECT_EQ(nextIndex, 0);
            auto account = createBitcoinLikeAccount(wallet, nextIndex, info);
            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiver);
            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                dispatcher->stop();
            });

            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);
            dispatcher->waitUntilStopped();
        }
    }
}