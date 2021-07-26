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
#include <functional>
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

namespace {
    const std::string kExplorerUrl = "https://xtz-explorer.api.live.ledger.com/explorer";
}

class TezosLikeWalletSynchronization : public BaseFixture {

};

TEST_F(TezosLikeWalletSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool("xtz", "");
    static std::function<void (
            const std::string &,
            const std::string &)> test = [=] (
                            const std::string &walletName,
                            const std::string &explorerURL) {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>'/<address>");
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, explorerURL);
        configuration->putString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);
        auto wallet = uv::wait(pool->createWallet(walletName, "tezos", configuration));
        {
            auto nextIndex = uv::wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            const auto externalExplorerUrl = [](const string& address) -> std::string {
                return fmt::format("https://tzstats.com/{}", address);
            };

            auto account = createTezosLikeAccount(wallet, nextIndex, XTZ_KEYS_INFO);
            const auto accountAddress = account->getKeychain()->getAddress()->toString();

            auto context = getTestExecutionContext();

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = uv::wait(account->getBalance());
                EXPECT_GT(balance->toLong(), 0L) << "The account should have a non-zero balance";

                auto originatedAccounts = account->getOriginatedAccounts();
                EXPECT_GE(originatedAccounts.size(), 2)
                        << fmt::format("{} should have at least 2 originated accounts. Check {}#contracts to make sure",
                                accountAddress,
                                externalExplorerUrl(accountAddress));

                for (auto &origAccount : originatedAccounts) {
                    const auto address = origAccount->getAddress();
                    std::cout << "Originated account: " << address << std::endl;

                    auto origOps = uv::wait(std::dynamic_pointer_cast<OperationQuery>(origAccount->queryOperations()->complete())->execute());
                    EXPECT_GE(origOps.size(), 3) << fmt::format("{} should have more than 3 ops. Check {} to make sure", address, externalExplorerUrl(address));
                    std::cout << ">>> Nb of originated ops: " << origOps.size() << std::endl;
                    auto origBalance = uv::wait(std::dynamic_pointer_cast<TezosLikeOriginatedAccount>(origAccount)->getBalance(dispatcher->getMainExecutionContext()));
                    EXPECT_GT(origBalance->toLong(), 0L) << fmt::format("{} should have a non-zero balance. Check {} to make sure", address, externalExplorerUrl(address)) ;
                    std::cout << ">>> Originated Balance: " << origBalance->toString() << std::endl;

                    auto fromDate = DateUtils::fromJSON("2019-02-01T13:38:23Z");
                    auto toDate = DateUtils::now();
                    auto balanceHistory = uv::wait(std::dynamic_pointer_cast<TezosLikeOriginatedAccount>(origAccount)->getBalanceHistory(dispatcher->getMainExecutionContext(), fromDate, toDate, api::TimePeriod::MONTH));
                    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), origBalance->toLong()); 
                }
                context->stop();
            });

            auto restoreKey = account->getRestoreKey();
            EXPECT_EQ(restoreKey, hex::toString(XTZ_KEYS_INFO.publicKeys[0]));
            auto bus = account->synchronize();
            bus->subscribe(context, receiver);

            context->waitUntilStopped();

            
            // re-launch a synchronization if it’s the first time
            std::cout << "Running a second synchronization." << std::endl;
            auto firstSyncOrigAccountsCount = account->getOriginatedAccounts().size();
            context = std::dynamic_pointer_cast<uv::SequentialExecutionContext>(
                dispatcher->getSerialExecutionContext("__second__"));
            auto bus2 = account->synchronize();
            bus2->subscribe(context, receiver);

            context->waitUntilStopped();

            auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            std::cout<<">>> Nb of ops: "<<ops.size()<<std::endl;
            EXPECT_GE(ops.size(), 5)
                        << fmt::format("{} should have at least 5 operations (as of 2021-07-21). Check {} to make sure",
                                accountAddress,
                                externalExplorerUrl(accountAddress));
            
            EXPECT_EQ(std::dynamic_pointer_cast<OperationApi>(ops[0])->asTezosLikeOperation()->getTransaction()->getStatus(), 1);
            auto fees = uv::wait(account->getFees());
            EXPECT_GT(fees->toUint64(), 0) << "Fees estimation should return a (strictly) positive number";

            auto storage = uv::wait(account->getStorage("tz1ZshTmtorFVkcZ7CpceCAxCn7HBJqTfmpk"));
            EXPECT_GT(storage->toUint64(), 0) << "Storage estimation for the account should return a (strictly) positive number";

            auto gasLimit = uv::wait(account->getEstimatedGasLimit("tz1ZshTmtorFVkcZ7CpceCAxCn7HBJqTfmpk"));
            EXPECT_GT(gasLimit->toUint64(), 0) << "Gas limit estimation should return a (strictly) positive number";

            // Making sure that both synchronizations found the same
            EXPECT_EQ(account->getOriginatedAccounts().size(), firstSyncOrigAccountsCount)
                    << "The account should have the same amount of originated accounts registered in 2 consecutive synchronizations";
        }
    };
    test("e847815f-488a-4301-b67c-378a5e9c8a61", kExplorerUrl);
    resetDispatcher();
    test("e847815f-488a-4301-b67c-378a5e9c8a60", kExplorerUrl);
}

TEST_F(TezosLikeWalletSynchronization, SynchronizeAccountWithMoreThan100OpsAndDeactivateSyncToken) {
    auto pool = newDefaultPool();
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>'/<address>");
    configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
    configuration->putBoolean(api::Configuration::DEACTIVATE_SYNC_TOKEN, true);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, kExplorerUrl);
    auto wallet = uv::wait(pool->createWallet("xtz", "tezos", configuration));
    auto account = createTezosLikeAccount(wallet, 0, XTZ_WITH_100_OPS_KEYS_INFO);
    auto bus = account->synchronize();
    bus->subscribe(account->getContext(), make_receiver([=](const std::shared_ptr<api::Event> &event) {
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED);
        dispatcher->stop();
    }));
    dispatcher->waitUntilStopped();
    auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations())->execute());
    EXPECT_GT(ops.size(), 100);
}

TEST_F(TezosLikeWalletSynchronization, NonActivated) {
    auto pool = newDefaultPool("xtz", "");
    auto configuration = DynamicObject::newInstance();

    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>'/<address>");
    configuration->putString(
        api::TezosConfiguration::TEZOS_XPUB_CURVE,
        api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519
    );
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, "https://api.tzstats.com/explorer");

    auto wallet = uv::wait(pool->createWallet("tezos wallet", "tezos", configuration));

    auto nextIndex = uv::wait(wallet->getNextAccountIndex());
    EXPECT_EQ(nextIndex, 0);

    auto account = createTezosLikeAccount(wallet, nextIndex, XTZ_NON_ACTIVATED_KEYS_INFO);

    auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
      fmt::print("Received event {}\n", api::to_string(event->getCode()));
      if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
        return;
      EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
      EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

      auto balance = uv::wait(account->getBalance());
      EXPECT_EQ(balance->toLong(), 0L);

      dispatcher->stop();
    });

    auto restoreKey = account->getRestoreKey();
    EXPECT_EQ(restoreKey, hex::toString(XTZ_NON_ACTIVATED_KEYS_INFO.publicKeys[0]));
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);

    dispatcher->waitUntilStopped();
}
