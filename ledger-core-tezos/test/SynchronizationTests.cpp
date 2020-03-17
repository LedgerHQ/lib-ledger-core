#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <set>

#include <core/api/BlockchainExplorerEngines.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/utils/DateUtils.hpp>

#include <tezos/TezosLikeCurrencies.hpp>
#include <tezos/TezosLikeWallet.hpp>
#include <tezos/api/TezosBlockchainExplorerEngines.hpp>
#include <tezos/api/TezosConfiguration.hpp>
#include <tezos/api/TezosConfigurationDefaults.hpp>
#include <tezos/database/TezosLikeAccountDatabaseHelper.hpp>
#include <tezos/delegation/TezosLikeOriginatedAccount.hpp>
#include <tezos/factories/TezosLikeWalletFactory.hpp>
#include <tezos/operations/TezosLikeOperation.hpp>
#include <tezos/operations/TezosLikeOperationQuery.hpp>
#include <tezos/transactions/TezosLikeTransactionBuilder.hpp>

#include <integration/WalletFixture.hpp>

#include "Common.hpp"

using namespace std;

class TezosLikeWalletSynchronization : public WalletFixture<TezosLikeWalletFactory> {
};

TEST_F(TezosLikeWalletSynchronization, MediumXpubSynchronization) {
    registerCurrency(currencies::tezos());

    static std::function<void (
        const std::string &,
        const std::string &,
        const std::string &)> test = [=] (
            const std::string &walletName,
            const std::string &nextWalletName,
            const std::string &explorerURL) {
        if (walletName.empty()) {
            return;
        }

        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>'/<address>");
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::TezosBlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, explorerURL);

         auto wallet = std::dynamic_pointer_cast<TezosLikeWallet>(wait(walletStore->createWallet(walletName, "tezos", configuration)));

        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto info = XTZ_KEYS_INFO;
            info.index = nextIndex;

            auto account = std::dynamic_pointer_cast<TezosLikeAccount>(wait(wallet->newAccountWithInfo(info)));

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

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
                    auto origOps = wait(std::dynamic_pointer_cast<TezosLikeOriginatedOperationQuery>(origAccount->queryOperations()->complete())->execute());
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

            auto ops = wait(std::dynamic_pointer_cast<TezosLikeOperationQuery>(account->queryOperations()->complete())->execute());
            std::cout<<">>> Nb of ops: " << ops.size()<<std::endl;
            EXPECT_GT(ops.size(), 0);

            EXPECT_EQ(std::dynamic_pointer_cast<TezosLikeOperation>(ops[0])->getTransaction()->getStatus(), 1);
            auto fees = wait(account->getFees());
            EXPECT_GT(fees->toUint64(), 0);

            auto storage = wait(account->getStorage("tz1ZshTmtorFVkcZ7CpceCAxCn7HBJqTfmpk"));
            EXPECT_GT(storage->toUint64(), 0);

            auto gasLimit = wait(account->getEstimatedGasLimit("tz1ZshTmtorFVkcZ7CpceCAxCn7HBJqTfmpk"));
            EXPECT_GT(gasLimit->toUint64(), 0);

            test(nextWalletName, "", explorerURL);
        }
    };

    // TODO: uncomment when our node works
    //test("e847815f-488a-4301-b67c-378a5e9c8a61", "e847815f-488a-4301-b67c-378a5e9c8a60", "https://xtz.explorers.prod.aws.ledger.fr/explorer");
    test("e847815f-488a-4301-b67c-378a5e9c8a61", "e847815f-488a-4301-b67c-378a5e9c8a60", "https://api.tzstats.com/explorer");
}
