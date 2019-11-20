/*
 *
 * tezos_synchronization
 *
 * Created by El Khalil Bellakrid on 11/05/2019.
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
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/transaction_builders/TezosLikeTransactionBuilder.h>
#include <iostream>
#include <api/BlockchainExplorerEngines.hpp>
#include <wallet/tezos/api_impl/TezosLikeOperation.h>
#include <wallet/tezos/delegation/TezosLikeOriginatedAccount.h>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>

using namespace std;

class TezosLikeWalletSynchronization : public BaseFixture {

};

TEST_F(TezosLikeWalletSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool("xtz", "");
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>'/<address>");
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "tezos", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createTezosLikeAccount(wallet, nextIndex, XTZ_KEYS_INFO);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = wait(account->getBalance());
                EXPECT_NE(balance->toLong(), 0L);

                auto originatedAccounts = account->getOriginatedAccounts();
                EXPECT_GE(originatedAccounts.size(), 2);

                for (auto &origAccount : originatedAccounts) {
                    auto origOps = wait(std::dynamic_pointer_cast<OperationQuery>(origAccount->queryOperations()->complete())->execute());
                    EXPECT_GE(origOps.size(), 3);
                    std::cout<<">>> Nb of originated ops: "<<origOps.size()<<std::endl;
                    auto origBalance = wait(std::dynamic_pointer_cast<TezosLikeOriginatedAccount>(origAccount)->getBalance(dispatcher->getMainExecutionContext()));
                    EXPECT_NE(origBalance->toLong(), 0L);
                    std::cout<<">>> Originated Balance: "<<origBalance->toString()<<std::endl;
                    auto fromDate = DateUtils::fromJSON("2019-02-01T13:38:23Z");
                    auto toDate = DateUtils::now();
                    auto balanceHistory = wait(std::dynamic_pointer_cast<TezosLikeOriginatedAccount>(origAccount)->getBalanceHistory(dispatcher->getMainExecutionContext(), fromDate, toDate, api::TimePeriod::MONTH));
                    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), origBalance->toLong());
                }

                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            EXPECT_EQ(restoreKey, hex::toString(XTZ_KEYS_INFO.publicKeys[0]));
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            dispatcher->waitUntilStopped();

            // re-launch a synchronization if it’s the first time
            std::cout << "Running a second synchronization." << std::endl;
            dispatcher = std::make_shared<QtThreadDispatcher>();
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            dispatcher->waitUntilStopped();

            auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            std::cout<<">>> Nb of ops: "<<ops.size()<<std::endl;
            EXPECT_GT(ops.size(), 0);

            EXPECT_EQ(std::dynamic_pointer_cast<OperationApi>(ops[0])->asTezosLikeOperation()->getTransaction()->getStatus(), 1);
            auto fees = wait(account->getFees());
            EXPECT_GT(fees->toUint64(), 0);

            auto storage = wait(account->getStorage("tz1ZshTmtorFVkcZ7CpceCAxCn7HBJqTfmpk"));
            EXPECT_GT(storage->toUint64(), 0);

            auto gasLimit = wait(account->getEstimatedGasLimit("tz1ZshTmtorFVkcZ7CpceCAxCn7HBJqTfmpk"));
            EXPECT_GT(gasLimit->toUint64(), 0);
        }
    }
}

