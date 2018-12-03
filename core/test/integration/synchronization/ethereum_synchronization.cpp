/*
 *
 * ethereum_synchronization
 *
 * Created by El Khalil Bellakrid on 27/07/2018.
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
#include <wallet/ethereum/database/EthereumLikeAccountDatabaseHelper.h>
#include <wallet/ethereum/transaction_builders/EthereumLikeTransactionBuilder.h>
#include <wallet/ethereum/ERC20/ERC20LikeAccount.h>

#include <iostream>
using namespace std;

class EthereumLikeWalletSynchronization : public BaseFixture {

};

TEST_F(EthereumLikeWalletSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>/<address>");
        //http://eth01.explorer.theory.rbx.ledger.fr:8104
        //http://eth01.explorer.theory.rbx.ledger.fr:21000
        //configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT,"http://18.202.239.45:20000");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION,"v2");
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ethereum", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createEthereumLikeAccount(wallet, nextIndex, ETH_MAIN_XPUB_INFO);

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
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = wait(account->getBalance());
                cout<<" ETH Balance: "<<balance->toLong()<<endl;
                auto txBuilder = std::dynamic_pointer_cast<EthereumLikeTransactionBuilder>(account->buildTransaction());
                auto erc20Accounts = account->getERC20Accounts();
                //EXPECT_EQ(erc20Accounts.size(), 1);
                //EXPECT_EQ(erc20Accounts[0]->getOperations().size(),3);
                //EXPECT_EQ(erc20Accounts[0]->getBalance()->intValue(), 1000);
                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            dispatcher->waitUntilStopped();

            auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            std::cout << "Ops: " << ops.size() << std::endl;

            auto block = wait(account->getLastBlock());
            auto blockHash = block.blockHash;
        }
    }
}

TEST_F(EthereumLikeWalletSynchronization, XpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        //configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>/<address>");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT,"http://eth-ropsten.explorers.dev.aws.ledger.fr");
        //configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION,"v2");
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ethereum_ropsten", configuration));
        //auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ethereum", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createEthereumLikeAccount(wallet, nextIndex, ETH_KEYS_INFO);
            auto keychain = account->getRestoreKey();

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            auto keyStore = account->getRestoreKey();

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = wait(account->getBalance());
                cout<<" ETH Balance: "<<balance->toLong()<<endl;
                auto txBuilder = std::dynamic_pointer_cast<EthereumLikeTransactionBuilder>(account->buildTransaction());
                auto erc20Accounts = account->getERC20Accounts();
                //EXPECT_EQ(erc20Accounts.size(), 1);
                //EXPECT_EQ(erc20Accounts[0]->getOperations().size(),3);
                //EXPECT_EQ(erc20Accounts[0]->getBalance()->intValue(), 1000);
                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            dispatcher->waitUntilStopped();

            auto opQuery = account->queryOperations()->complete();
            auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            //auto ops = ::wait(std::dynamic_pointer_cast<OperationQuery>(opQuery)->execute());
            std::cout << "Ops: " << ops.size() << std::endl;

            auto block = wait(account->getLastBlock());
            auto blockHash = block.blockHash;
        }
    }
}

