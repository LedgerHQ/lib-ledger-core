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
#include "../BaseFixture.h"
#include <set>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <api/KeychainEngines.hpp>
#include <api/PoolConfiguration.hpp>
class BitcoinLikeWalletSynchronization : public BaseFixture {

};

TEST_F(BitcoinLikeWalletSynchronization, MediumXpubSynchronization) {
    auto configuration = DynamicObject::newInstance();
#ifdef PG_SUPPORT
    const bool usePostgreSQL = true;
    configuration->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
    auto pool = newDefaultPool("postgres", "", configuration, usePostgreSQL);
#else
    auto pool = newDefaultPool();
#endif

    {
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP173_P2WPKH);
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT,"https://bitcoin-mainnet.explorers.dev.aws.ledger.fr:443");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION, "v3");
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin",
                                              configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2WPKH_MEDIUM_XPUB_INFO);

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
                auto balance = wait(account->getBalance())->toString();
                auto destination = "bc1qh4kl0a0a3d7su8udc2rn62f8w939prqpl34z86";
                auto txBuilder = account->buildTransaction(false);
                auto tx = wait(std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(txBuilder->sendToAddress(api::Amount::fromLong(wallet->getCurrency(), 2000), destination)
                                                                                                    ->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF)
                                                                                                    ->setFeesPerByte(api::Amount::fromLong(wallet->getCurrency(), 50)))
                                       ->build());
                EXPECT_EQ(tx->getOutputs()[0]->getAddress().value_or(""), destination);

                auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
                std::cout << "Ops: " << ops.size() << std::endl;
                for (auto& op : ops) {
                    std::cout << "op: " << op->asBitcoinLikeOperation()->getTransaction()->getHash() << std::endl;
                    std::cout << " amount: " << op->getAmount()->toLong() << std::endl;
                    std::cout << " type: " << api::to_string(op->getOperationType()) << std::endl;
                }
                dispatcher->stop();
            });

            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            dispatcher->waitUntilStopped();
        }
    }
}

TEST_F(BitcoinLikeWalletSynchronization, MediumDGBXpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP173_P2WPKH);
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "digibyte", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2WPKH_DGB_MEDIUM_KEYS_INFO);

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
                EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "dgb1qgdg3hdysnpmaxpdpqqzhey2f5888av488hq0z6");
                dispatcher->stop();
            });

            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            dispatcher->waitUntilStopped();
        }
    }
}

TEST_F(BitcoinLikeWalletSynchronization, MediumLTCXpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP173_P2WPKH);
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "litecoin", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2WPKH_LTC_MEDIUM_KEYS_INFO);

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
                EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "ltc1q7qnj9xm8wp8ucmg64lk0h03as8k6ql6rk4wvsd");
                dispatcher->stop();
            });

            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);

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

TEST_F(BitcoinLikeWalletSynchronization, SynchronizeAndFreshResetAll) {
    {
        auto pool = newDefaultPool();
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

            // reset everything
            EXPECT_EQ(wait(pool->getWalletCount()), 1);
            wait(pool->freshResetAll());
        }
    }
    {
        auto pool = newDefaultPool();
        EXPECT_EQ(wait(pool->getWalletCount()), 0);
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
            EXPECT_EQ(bus, account->synchronize());
            dispatcher->waitUntilStopped();
            return bus;
        };
        auto b1 = synchronize(true);
        auto b2 = synchronize(false);
        EXPECT_NE(b1, b2);
    }
}

TEST_F(BitcoinLikeWalletSynchronization, TestNetSynchronization) {
    auto pool = newDefaultPool();
    {
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin_testnet",
                                              api::DynamicObject::newInstance()));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());
            info.extendedKeys.push_back("tpubDCJarhe7f951cUufTWeGKh1w6hDgdBcJfvQgyMczbxWvwvLdryxZuchuNK3KmTKXwBNH6Ze6tHGrUqvKGJd1VvSZUhTVx58DrLn9hR16DVr");
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
                //EXPECT_EQ(event->getCode(),
                //          api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                auto amount = wait(account->getBalance());
                auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
                std::cout << "Amount: " << amount->toLong() << std::endl;
                std::cout << "Ops: " << ops.size() << std::endl;
                for (auto& op : ops) {
                    std::cout << "op: " << op->asBitcoinLikeOperation()->getTransaction()->getHash() << std::endl;
                    std::cout << " amount: " << op->getAmount()->toLong() << std::endl;
                    std::cout << " type: " << api::to_string(op->getOperationType()) << std::endl;
                }


                // Test UTXOs
                auto utxoCount = wait(account->getUTXOCount());
                auto utxos = wait(account->getUTXO(0, 1000000));
                EXPECT_EQ(utxos.size(), utxoCount);

                dispatcher->stop();
            });

            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            dispatcher->waitUntilStopped();
        }
    }
}

TEST_F(BitcoinLikeWalletSynchronization, MultipleSynchronization) {
    auto pool = newDefaultPool();
    {
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin",
                                              api::DynamicObject::newInstance()));
        std::vector<std::string> xpubs = {
                P2PKH_MEDIUM_XPUB_INFO.extendedKeys[0]
        };
        std::vector<std::shared_ptr<BitcoinLikeAccount>> accounts;
        for (auto x : xpubs) {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());
            info.extendedKeys.push_back(x);
            auto account = createBitcoinLikeAccount(wallet, nextIndex, info);
            accounts.push_back(account);
        }

        std::set<std::string> emittedOperations;
        {
            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            static std::function<void (int)> syncAccount = [accounts, receiver, this](int index){
                auto localReceiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                    fmt::print("Received event {}\n", api::to_string(event->getCode()));
                    if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                        return;
                    EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                    auto amount = wait(accounts[index]->getBalance());
                    fmt::print("Amount: {}\n", amount->toLong());
                    if (index == accounts.size() - 1) {
                        dispatcher->stop();
                    }
                });

                accounts[index]->synchronize()->subscribe(dispatcher->getMainExecutionContext(), localReceiver);
                if (index == accounts.size() - 1) {
                    dispatcher->waitUntilStopped();
                } else {
                    syncAccount(index + 1);
                }
            };
            syncAccount(0);
        }
    }
}

TEST_F(BitcoinLikeWalletSynchronization, SynchronizationAfterErase) {
    auto pool = newDefaultPool();
    {
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin",
                                              api::DynamicObject::newInstance()));

        auto nextIndex = wait(wallet->getNextAccountIndex());
        auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());
        info.extendedKeys.push_back(P2PKH_MEDIUM_XPUB_INFO.extendedKeys[0]);
        auto account = createBitcoinLikeAccount(wallet, nextIndex, info);

        std::set<std::string> emittedOperations;
        {
            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            auto date = "2000-03-27T09:10:22Z";
            auto formatedDate = DateUtils::fromJSON(date);

            static std::function<void (bool)> syncAccount = [formatedDate, account, this](bool shouldStop) {
                auto localReceiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                    fmt::print("Received event {}\n", api::to_string(event->getCode()));
                    if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                        return;
                    EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                    auto amount = wait(account->getBalance());
                    fmt::print("Amount: {}\n", amount->toLong());

                    auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
                    EXPECT_GT(ops.size(), 0);

                    //Delete account
                    auto code = wait(account->eraseDataSince(formatedDate));
                    EXPECT_EQ(code, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

                    ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
                    EXPECT_EQ(ops.size(), 0);

                    if (shouldStop) {
                        dispatcher->stop();
                    } else {
                        syncAccount(true);
                    }
                });

                account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), localReceiver);
                dispatcher->waitUntilStopped();
                return Future<Unit>::successful(unit);
            };
            syncAccount(false);
        }
    }
}
TEST_F(BitcoinLikeWalletSynchronization, BTCParsingAndSerialization) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("testnet_wallet", "bitcoin", DynamicObject::newInstance()));
    {
        auto strTx = "0100000001c76ec87ab18aa0398a2cbfa68625576fdc3bf276b467fc016010ad675678157d010000006b483045022100e0c4a6449841f4a435b23dc2cd4a6c26a8e12e25783dbd02072332c794012ca202202246876625e726ef9a89f854d59d2aee1787c9807d82d953732e56dbc296657001210212b8ae5848c5ce1422643aed011c9b1cbb7da9a5feba0cad0c130a11e8c4091dffffffff02400d03000000000017a914b800848ce7130e91d55422e1f3d72e813dc250e187b9dc2b00000000001976a9140265b33d266d56c25416d493ccb42992faa3f24a88ac00000000";
        auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
    }
}

TEST_F(BitcoinLikeWalletSynchronization, XSTParsingAndSerialization) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("testnet_wallet", "stealthcoin", DynamicObject::newInstance()));
    {
        auto strTx = "01000000ae71115b01f9d2e90eef51048962392b2ac2cbe476aacb06d750e0794aeb5da5a2afaf19da000000006b483045022100be2faab00cc32a4f6f0249d70f3b6d1fc66bf50dcd6ae011d0287a4a581e5c7a02203cd71132619de1e8a1a84c481529f32d4e29b495f835d5b0da7655203a0a7dda01210269568d231762330f7aed9cf0acfb2512f2d7889eb18adb778589ab5cca66fb3dffffffff0200127a00000000001976a914b4949cd1e6c07826ceee84929a7c6babcccc5ec388ac6094b500000000001976a91417cb2228c292d617f98f4b89b448650e0a480e0788ac00000000";
        auto txHash = "38fcb406a0110c50465edb482bab8d6100e7f9fa7e3ae01e48145c60fd51d00b";
        auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
        EXPECT_EQ(tx->getHash(), txHash);
    }
    {
        auto strTx = "020000000162cd1e8fd9fe9e07a27c969a4fdc74adcb3d01f8b62b918998bc7971070fb1000100000048473044022071051379723e794e5e1f4931755106b6c5fa0ab0b1f8e5fc1a77241d14c428c6022020748197e9d00b379307e0eb2a22d419815faaf96e342ba86d8931b05ac03ab701ffffffff02000000000000000000977ae027000000002321032ce5cc649a30eb4f052bc2ff2080c53781ddb0881a4469f77e060c91d32671f5ac00000000";
        auto txHash = "c7578a4909c7000403df354ec8ce4a8a7f9074c935c45491f74783fd1cc03c0e";
        auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
        EXPECT_EQ(tx->getHash(), txHash);
    }
}
