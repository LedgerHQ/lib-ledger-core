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
#include <api/ConfigurationDefaults.hpp>
#include <api/KeychainEngines.hpp>
#include <api/EthereumLikeTransaction.hpp>
#include <api/OperationOrderKey.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/ethereum/database/EthereumLikeAccountDatabaseHelper.h>
#include <wallet/ethereum/transaction_builders/EthereumLikeTransactionBuilder.h>
#include <wallet/ethereum/ERC20/ERC20LikeAccount.h>
#include <iostream>
#include "FakeHttpClient.hpp"
#include "../../fixtures/http_cache_EthereumLikeWalletSynchronization_MediumXpubSynchronization_1.h"
#include "../../fixtures/http_cache_EthereumLikeWalletSynchronization_MediumXpubSynchronization_2.h"
#include "../../fixtures/http_cache_EthereumLikeWalletSynchronization_MediumXpubSynchronization_3.h"
#include "../../fixtures/http_cache_EthereumLikeWalletSynchronization_MediumXpubSynchronization_4.h"

using namespace std;

class EthereumLikeWalletSynchronization : public BaseFixture {

};

TEST_F(EthereumLikeWalletSynchronization, MediumXpubSynchronization) {
    http->addCache(HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_1::URL,
        HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_1::BODY);
    http->addCache(HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_2::URL,
        HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_2::BODY);
    http->addCache(HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_3::URL,
        HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_3::BODY);
    http->addCache(HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_4::URL,
        HTTP_CACHE_EthereumLikeWalletSynchronization_MediumXpubSynchronization_4::BODY);
    
    auto walletName = "e847815f-488a-4301-b67c-378a5e9c8a61";
    auto erc20Count = 0;
    {
        auto pool = newDefaultPool();
        {
            auto configuration = DynamicObject::newInstance();
            configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/60'/0'/0/<account>'");
            configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT,"https://explorers.api.live.ledger.com");
            auto wallet = uv::wait(pool->createWallet(walletName, "ethereum", configuration));
            std::set<std::string> emittedOperations;
            {
                auto nextIndex = uv::wait(wallet->getNextAccountIndex());
                EXPECT_EQ(nextIndex, 0);

                auto account = createEthereumLikeAccount(wallet, nextIndex, ETH_KEYS_INFO_LIVE);
                auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                    if (event->getCode() == api::EventCode::NEW_OPERATION) {
                        auto uid = event->getPayload()->getString(
                                api::Account::EV_NEW_OP_UID).value();
                        EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                    }
                });

                auto eventBus = pool->getEventBus();
                eventBus->subscribe(getTestExecutionContext(),receiver);

                receiver.reset();
                receiver = make_receiver([=, &erc20Count](const std::shared_ptr<api::Event> &event) {
                    fmt::print("Received event {}\n", api::to_string(event->getCode()));
                    if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                        return;
                    EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                    EXPECT_EQ(event->getCode(),
                              api::EventCode::SYNCHRONIZATION_SUCCEED);

                    auto balance =uv::wait(account->getBalance());

                    auto erc20Accounts = account->getERC20Accounts();
                    erc20Count = erc20Accounts.size();
                    EXPECT_NE(erc20Count, 0);
                    std::cout << "ERC20 Accounts: " << erc20Count << std::endl;

                    auto erc20Ops = erc20Accounts[0]->getOperations();
                    EXPECT_NE(erc20Ops.size(), 0);
                    EXPECT_NE(erc20Ops[0]->getBlockHeight().value_or(0), 0);

                    auto erc20Balance = uv::wait(std::dynamic_pointer_cast<ERC20LikeAccount>(erc20Accounts[0])->getBalance());
                    auto erc20BalanceFromAccount = uv::wait(account->getERC20Balance(erc20Accounts[0]->getToken().contractAddress));
                    EXPECT_EQ(erc20Balance->toString(10), erc20BalanceFromAccount->toString(10));

                    std::vector<std::string> erc20Addresses;
                    for (auto &erc20Acc: erc20Accounts) {
                        erc20Addresses.push_back(erc20Acc->getToken().contractAddress);
                    }
                    auto erc20BalancesFromAccount = uv::wait(account->getERC20Balances(erc20Addresses));
                    EXPECT_EQ(erc20BalancesFromAccount.size(), erc20Accounts.size());
                    EXPECT_EQ(erc20BalancesFromAccount[0]->toString(10), erc20Balance->toString(10));

                    auto amountToSend = std::make_shared<api::BigIntImpl>(BigInt::fromString("10"));
                    auto transferData = uv::wait(std::dynamic_pointer_cast<ERC20LikeAccount>(erc20Accounts[0])->getTransferToAddressData(amountToSend, "0xabf06640f8ca8fC5e0Ed471b10BeFCDf65A33e43"));
                    EXPECT_GT(transferData.size(), 0);

                    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(erc20Accounts[0]
                                                                                             ->queryOperations()
                                                                                             ->addOrder(api::OperationOrderKey::DATE, true)
                                                                                             ->complete())->execute());
                    std::cout << "ERC20 Operations: " << operations.size() << std::endl;
                    EXPECT_NE(operations.size(), 0);

                    int64_t gasPrice = 20, gasLimit = 200;
                    auto currency = account->getWallet()->getCurrency();
                    auto txBuilder = std::dynamic_pointer_cast<EthereumLikeTransactionBuilder>(account->buildTransaction());
                    txBuilder->setGasPrice(api::Amount::fromLong(currency, 20));
                    txBuilder->setGasLimit(api::Amount::fromLong(currency, 200));
                    txBuilder->wipeToAddress("0xfb98bdd04d82648f25e67041d6e27a866bec0b47");
                    auto tx = uv::wait(txBuilder->build());
                    EXPECT_EQ(tx->getValue()->toLong(), balance->toLong() - (gasLimit * gasPrice));

                    dispatcher->stop();
                });

                auto restoreKey = account->getRestoreKey();
                auto eventBus2 = account->synchronize();
                eventBus2->subscribe(getTestExecutionContext(),receiver);

                dispatcher->waitUntilStopped();

                auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
                std::cout << "Ops: " << ops.size() << std::endl;

                auto block = uv::wait(account->getLastBlock());
                auto blockHash = block.blockHash;
            }
        }
    }
    // Recover account
    {
        resetDispatcher();
        auto pool = newDefaultPool();
        auto wallet = uv::wait(pool->getWallet(walletName));
        auto account = std::dynamic_pointer_cast<EthereumLikeAccount>(uv::wait(wallet->getAccount(0)));
        EXPECT_EQ(account->getERC20Accounts().size(), erc20Count);
        uv::wait(pool->freshResetAll());
    }
}

TEST_F(EthereumLikeWalletSynchronization, BalanceHistory) {
    auto walletName = "e847815f-488a-4301-b67c-378a5e9c8a61";
    auto erc20Count = 0;
    {
        auto pool = newDefaultPool();
        {
            auto configuration = DynamicObject::newInstance();
            configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/60'/0'/0/<account>'");
            configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT,"https://explorers.api.live.ledger.com");
            auto wallet = uv::wait(pool->createWallet(walletName, "ethereum", configuration));

            {
                std::string balanceStr;
                auto nextIndex = uv::wait(wallet->getNextAccountIndex());
                auto account = createEthereumLikeAccount(wallet, nextIndex, ETH_KEYS_INFO_LIVE);

                auto receiver = make_receiver([=, &erc20Count, &balanceStr](const std::shared_ptr<api::Event> &event) {
                    fmt::print("Received event {}\n", api::to_string(event->getCode()));

                    if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                        return;
                    
                    auto balance = uv::wait(account->getBalance());
                    balanceStr = balance->toString();

                    auto now = std::time(nullptr);
                    char now_str[256];
                    std::strftime(now_str, sizeof(now_str), "%Y-%m-%dT%H:%M:%SZ", std::localtime(&now));

                    auto history = uv::wait(account->getBalanceHistory(
                        "2019-09-20T00:00:00Z",
                        now_str,
                        api::TimePeriod::DAY
                    ));

                    EXPECT_EQ(history.back()->toString(), balanceStr);
                    
                    dispatcher->stop();
                });

                auto eventBus = account->synchronize();
                eventBus->subscribe(getTestExecutionContext(), receiver);
                
                dispatcher->waitUntilStopped();
            }
        }
    }
}

TEST_F(EthereumLikeWalletSynchronization, DISABLED_XpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/60'/0'/0/<account>'");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT,"https://explorers.api.live.ledger.com");
        auto wallet = uv::wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ethereum", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = uv::wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createEthereumLikeAccount(wallet, nextIndex, ETH_KEYS_INFO_LIVE);
            auto keychain = account->getRestoreKey();

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            auto keyStore = account->getRestoreKey();

            auto eventBus = pool->getEventBus();
            eventBus->subscribe(dispatcher->getMainExecutionContext(),receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = uv::wait(account->getBalance());
                cout<<" ETH Balance: "<<balance->toLong()<<endl;
                auto txBuilder = std::dynamic_pointer_cast<EthereumLikeTransactionBuilder>(account->buildTransaction());
                auto erc20Accounts = account->getERC20Accounts();
                EXPECT_GT(erc20Accounts.size(), 0);
                EXPECT_GT(erc20Accounts[0]->getOperations().size(),0);
                auto erc20Balance = uv::wait(std::dynamic_pointer_cast<ERC20LikeAccount>(erc20Accounts[0])->getBalance());
                EXPECT_TRUE(BigInt(erc20Balance->toString(10)) > BigInt("0"));
                auto contractAddress = erc20Accounts[0]->getToken().contractAddress;
                std::cout << "Contract Address: " << contractAddress << std::endl;
                std::cout << "ERC20 balance: " << erc20Balance->toString(10) << std::endl;
                auto erc20Ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(erc20Accounts[0]->queryOperations()->complete())->execute());
                EXPECT_EQ(erc20Accounts[0]->getOperations().size(), erc20Ops.size());
                EXPECT_EQ(erc20Ops[0]->isComplete(), true);
                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            auto eventBus2 = account->synchronize();
            eventBus2->subscribe(getTestExecutionContext(),receiver);

            dispatcher->waitUntilStopped();

            auto opQuery = account->queryOperations()->complete();
            auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            //auto ops = ::wait(std::dynamic_pointer_cast<OperationQuery>(opQuery)->execute());
            std::cout << "Ops: " << ops.size() << std::endl;
            EXPECT_EQ(ops[0]->isComplete(), true);
            auto block = uv::wait(account->getLastBlock());
            auto blockHash = block.blockHash;
        }
    }
}

TEST_F(EthereumLikeWalletSynchronization, XpubETCSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/60'/0'/<account>");
        auto wallet = uv::wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ethereum_classic", configuration));
        std::set<std::string> emittedOperations;
        {
            auto infos = uv::wait(wallet->getNextAccountCreationInfo());
            EXPECT_EQ(infos.index, 0);

            infos.publicKeys = ETC_KEYS_INFO_LIVE.publicKeys;
            infos.chainCodes = ETC_KEYS_INFO_LIVE.chainCodes;

            // Test that the cointype we use is the one set by KEYCHAIN_DERIVATION_SCHEME
            EXPECT_EQ(infos.derivations[0], "44'/60'/0'/0");
            auto account = createEthereumLikeAccount(wallet, infos.index, infos);

            auto keychain = account->getRestoreKey();

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            auto keyStore = account->getRestoreKey();

            auto eventBus = pool->getEventBus();
            eventBus->subscribe(dispatcher->getMainExecutionContext(),receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = uv::wait(account->getBalance());
                cout<<" ETH Balance: "<<balance->toLong()<<endl;
                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            auto eventBus2 = account->synchronize();
            eventBus2->subscribe(getTestExecutionContext(),receiver);

            dispatcher->waitUntilStopped();

            auto opQuery = account->queryOperations()->complete();
            auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            std::cout << "Ops: " << ops.size() << std::endl;
            EXPECT_EQ(ops[0]->isComplete(), true);
        }
    }
}

std::pair<std::shared_ptr<LambdaEventReceiver>, ledger::core::Future<bool>> createSyncReceiver() {
    auto promise = std::make_shared<Promise<bool>>();
    return
        std::make_pair<std::shared_ptr<LambdaEventReceiver>, ledger::core::Future<bool>>(
            make_receiver([promise](const std::shared_ptr<api::Event>& event) {
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED ||
                    event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT) {
                    promise->success(true);
                }
                else if (event->getCode() == api::EventCode::SYNCHRONIZATION_FAILED) {
                    promise->success(false);
                }
            }),
            promise->getFuture());
}

TEST_F(EthereumLikeWalletSynchronization, ReorgLastBlock) {
    auto walletName = "e847815f-488a-4301-b67c-378a5e9c8a61";
    {
        auto fakeHttp = std::make_shared<test::FakeHttpClient>();

        backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());

        auto pool = WalletPool::newInstance(
            "my_ppol",
            "",
            fakeHttp,
            ws,
            resolver,
            printer,
            dispatcher,
            rng,
            backend,
            api::DynamicObject::newInstance(),
            nullptr,
            nullptr
        );
        {
            auto configuration = DynamicObject::newInstance();
            configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/60'/0'/0/<account>'");
            configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, "http://test.test");
            auto wallet = uv::wait(pool->createWallet(walletName, "ethereum", configuration));
            {
                auto nextIndex = uv::wait(wallet->getNextAccountIndex());
                EXPECT_EQ(nextIndex, 0);

                auto account = createEthereumLikeAccount(wallet, nextIndex, ETH_KEYS_INFO_LIVE);
                auto waiter = createSyncReceiver();
                test::UrlConnectionData blockNotFound;
                blockNotFound.statusCode = 404;
                blockNotFound.body = "Block 0xaef3a92f1017445c98139b7ab3ddd6e8abfe75588652338ca6f537cf47ab620d not found";

                fakeHttp->setBehavior({
                    {
                        "http://test.test/blockchain/v3/eth/blocks/current" ,
                        test::FakeUrlConnection::fromString("{\"hash\":\"0xdd73566cf4913cba5060377397283cf411e03bdbe1b83d4695699d9e6ac1c941\",\"height\":8175344,\"time\":\"2019-07-18T14:54:00Z\",\"txs\":[]}")
                    },
                    {
                        "http://test.test/blockchain/v3/eth/syncToken",
                        test::FakeUrlConnection::fromString("{\"token\":\"f867afdf-c5b1-4a9d-b3e3-b4dfc985be8a\"}")
                    },
                    {
                        "http://test.test/blockchain/v3/eth/addresses/0xabf06640f8ca8fC5e0Ed471b10BeFCDf65A33e43/transactions",
                        test::FakeUrlConnection::fromString("\
                            {\
                                \"truncated\": false,\
                                \"txs\": [\
                                    {\
                                        \"hash\": \"0x24ebd24caebcdb5d5fe22ba4d2e1904515dbb7d8fcd968789c8810ad379ba866\",\
                                        \"status\": 1,\
                                        \"received_at\": \"2018-06-08T10:19:10Z\",\
                                        \"nonce\": \"0x05\",\
                                        \"value\": 1000000000000000,\
                                        \"gas\": 21000,\
                                        \"gas_price\": 11000000000,\
                                        \"from\": \"0xf0c91a8f0f95a2fd90ab74c6c13ef0db2af96447\",\
                                        \"to\": \"0xabf06640f8ca8fC5e0Ed471b10BeFCDf65A33e43\",\
                                        \"input\": \"0x\",\
                                        \"cumulative_gas_used\": 6388806,\
                                        \"gas_used\": 21000,\
                                        \"transfer_events\": {\
                                            \"list\": [],\
                                            \"truncated\": false\
                                        },\
                                        \"actions\": [\
                                            {\
                                                \"from\": \"0xf0c91a8f0f95a2fd90ab74c6c13ef0db2af96447\",\
                                                \"to\": \"0xabf06640f8ca8fC5e0Ed471b10BeFCDf65A33e43\",\
                                                \"value\": 1000000000000000\
                                            }\
                                        ],\
                                        \"block\": {\
                                            \"hash\": \"0x0160a3b2b762ec42f5a1134ac260a21c98f377e153d332efb6e20534abb37f78\",\
                                            \"height\": 5752778,\
                                            \"time\": \"2018-06-08T10:19:10Z\"\
                                        }\
                                    }\
                                ]\
                            }")
                    },
                    {
                        "http://test.test/blockchain/v3/eth/addresses/0xabf06640f8ca8fC5e0Ed471b10BeFCDf65A33e43/transactions?block_hash=0x0160a3b2b762ec42f5a1134ac260a21c98f377e153d332efb6e20534abb37f78",
                        std::make_shared<test::FakeUrlConnection>(blockNotFound)
                    }
                    });
                auto eventBus = account->synchronize();
                eventBus->subscribe(getTestExecutionContext(), waiter.first);
                EXPECT_TRUE(uv::wait(waiter.second));
                // next time
                waiter = createSyncReceiver();
                auto eventBus2 = account->synchronize();
                eventBus2->subscribe(getTestExecutionContext(), waiter.first);
                EXPECT_TRUE(uv::wait(waiter.second));
                dispatcher->stop();
            }
        }
    }
}
