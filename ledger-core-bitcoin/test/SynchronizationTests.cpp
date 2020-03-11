/*
 *
 * SynchronizationTests
 * ledger-core-bitcoin
 *
 * Created by Alexis Le Provost on 10/02/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include <set>

#include <gtest/gtest.h>

#include <core/api/ConfigurationDefaults.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/api/PoolConfiguration.hpp>

#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/database/BitcoinLikeAccountDatabaseHelper.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>
#include <bitcoin/transactions/BitcoinLikeTransaction.hpp>
#include <bitcoin/transactions/BitcoinLikeTransactionBuilder.hpp>
#include <bitcoin/operations/BitcoinLikeOperationQuery.hpp>

#include <integration/WalletFixture.hpp>

#include "fixtures/Fixtures.hpp"

class BitcoinSynchronizations : public WalletFixture<BitcoinLikeWalletFactory> {

};

using namespace ledger::testing;

TEST_F(BitcoinSynchronizations, MediumXpubSynchronization) {
    auto const currency = currencies::bitcoin();
    auto configuration = api::DynamicObject::newInstance();

    if (DatabaseBackend::isPostgreSQLSupported()) {
        backend = std::static_pointer_cast<DatabaseBackend>(
                DatabaseBackend::getPostgreSQLBackend(api::ConfigurationDefaults::DEFAULT_PG_CONNECTION_POOL_SIZE));
        configuration->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
        services = newDefaultServices(DEFAULT_TENANT, DEFAULT_PASSWORD, configuration);
        walletStore = newWalletStore(services);
    }

    registerCurrency(currency);

    {
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP173_P2WPKH);
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, "https://bitcoin-mainnet.explorers.dev.aws.ledger.fr:443");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION, "v3");

        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", currency.name, configuration));

        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, P2WPKH_MEDIUM_XPUB_INFO);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiver);

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

                auto ops = wait(std::dynamic_pointer_cast<BitcoinLikeOperationQuery>(account->queryOperations()->complete())->execute());
                std::cout << "Ops: " << ops.size() << std::endl;
                for (auto& op : ops) {
                    std::cout << "op: " << std::dynamic_pointer_cast<BitcoinLikeOperation>(op)->getTransaction()->getHash() << std::endl;
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

TEST_F(BitcoinSynchronizations, SynchronizeOnceAtATime) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    {
        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a62", currency.name, api::DynamicObject::newInstance()));
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, P2PKH_MEDIUM_XPUB_INFO);
            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),
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

TEST_F(BitcoinSynchronizations, SynchronizeAndFreshResetAll) {
    auto const currency = currencies::bitcoin();

    {

        registerCurrency(currency);

        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a62", currency.name, api::DynamicObject::newInstance()));
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, P2PKH_MEDIUM_XPUB_INFO);
            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),
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
            EXPECT_EQ(wait(walletStore->getWalletCount()), 1);
            services->freshResetAll();
        }
    }
    {
        SetUp();

        EXPECT_EQ(wait(walletStore->getWalletCount()), 0);
    }
}

TEST_F(BitcoinSynchronizations, SynchronizeFromLastBlock) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);
    {
        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a63", currency.name, api::DynamicObject::newInstance()));

        createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);

        auto synchronize = [wallet, this] (bool expectNewOp) {
            auto account = wait(wallet->getAccount(0));
            auto numberOfOp = 0;

            auto receiverNumberOp = make_receiver([&numberOfOp](const std::shared_ptr<api::Event> &event) {
                numberOfOp += 1;
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiverNumberOp);

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

TEST_F(BitcoinSynchronizations, TestNetSynchronization) {
    auto const currency = currencies::bitcoin_testnet();

    registerCurrency(currency);

    {
        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", currency.name, api::DynamicObject::newInstance()));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());
            info.extendedKeys.push_back("tpubDCJarhe7f951cUufTWeGKh1w6hDgdBcJfvQgyMczbxWvwvLdryxZuchuNK3KmTKXwBNH6Ze6tHGrUqvKGJd1VvSZUhTVx58DrLn9hR16DVr");
            EXPECT_EQ(nextIndex, 0);
            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, info);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiver);

            receiver.reset();

            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                //EXPECT_EQ(event->getCode(),
                //          api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                auto amount = wait(account->getBalance());
                auto ops = wait(std::dynamic_pointer_cast<BitcoinLikeOperationQuery>(account->queryOperations()->complete())->execute());
                std::cout << "Amount: " << amount->toLong() << std::endl;
                std::cout << "Ops: " << ops.size() << std::endl;
                for (auto& op : ops) {
                    std::cout << "op: " << std::dynamic_pointer_cast<BitcoinLikeOperation>(op)->getTransaction()->getHash() << std::endl;
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

TEST_F(BitcoinSynchronizations, MultipleSynchronization) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    {
        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", currency.name, api::DynamicObject::newInstance()));
        std::vector<std::string> xpubs = { P2PKH_MEDIUM_XPUB_INFO.extendedKeys[0] };
        std::vector<std::shared_ptr<BitcoinLikeAccount>> accounts;

        for (auto x : xpubs) {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());
            info.extendedKeys.push_back(x);
            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, info);
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

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

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

TEST_F(BitcoinSynchronizations, SynchronizationAfterErase) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    {
        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", currency.name, api::DynamicObject::newInstance()));
        auto nextIndex = wait(wallet->getNextAccountIndex());
        auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());

        info.extendedKeys.push_back(P2PKH_MEDIUM_XPUB_INFO.extendedKeys[0]);

        auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, info);

        std::set<std::string> emittedOperations;
        {
            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

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

                    auto ops = wait(std::dynamic_pointer_cast<BitcoinLikeOperationQuery>(account->queryOperations()->complete())->execute());
                    EXPECT_GT(ops.size(), 0);

                    //Delete account
                    auto code = wait(account->eraseDataSince(formatedDate));
                    EXPECT_EQ(code, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

                    ops = wait(std::dynamic_pointer_cast<BitcoinLikeOperationQuery>(account->queryOperations()->complete())->execute());
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


TEST_F(BitcoinSynchronizations, BTCParsingAndSerialization) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("testnet_wallet", currency.name, api::DynamicObject::newInstance()));
    {
        auto strTx = "0100000001c76ec87ab18aa0398a2cbfa68625576fdc3bf276b467fc016010ad675678157d010000006b483045022100e0c4a6449841f4a435b23dc2cd4a6c26a8e12e25783dbd02072332c794012ca202202246876625e726ef9a89f854d59d2aee1787c9807d82d953732e56dbc296657001210212b8ae5848c5ce1422643aed011c9b1cbb7da9a5feba0cad0c130a11e8c4091dffffffff02400d03000000000017a914b800848ce7130e91d55422e1f3d72e813dc250e187b9dc2b00000000001976a9140265b33d266d56c25416d493ccb42992faa3f24a88ac00000000";
        auto tx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
    }
}

TEST_F(BitcoinSynchronizations, XSTParsingAndSerialization) {
    auto const currency = currencies::stealthcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("testnet_wallet", currency.name, api::DynamicObject::newInstance()));
    {
        auto strTx = "01000000ae71115b01f9d2e90eef51048962392b2ac2cbe476aacb06d750e0794aeb5da5a2afaf19da000000006b483045022100be2faab00cc32a4f6f0249d70f3b6d1fc66bf50dcd6ae011d0287a4a581e5c7a02203cd71132619de1e8a1a84c481529f32d4e29b495f835d5b0da7655203a0a7dda01210269568d231762330f7aed9cf0acfb2512f2d7889eb18adb778589ab5cca66fb3dffffffff0200127a00000000001976a914b4949cd1e6c07826ceee84929a7c6babcccc5ec388ac6094b500000000001976a91417cb2228c292d617f98f4b89b448650e0a480e0788ac00000000";
        auto txHash = "38fcb406a0110c50465edb482bab8d6100e7f9fa7e3ae01e48145c60fd51d00b";
        auto tx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
        EXPECT_EQ(tx->getHash(), txHash);
    }
    {
        auto strTx = "020000000162cd1e8fd9fe9e07a27c969a4fdc74adcb3d01f8b62b918998bc7971070fb1000100000048473044022071051379723e794e5e1f4931755106b6c5fa0ab0b1f8e5fc1a77241d14c428c6022020748197e9d00b379307e0eb2a22d419815faaf96e342ba86d8931b05ac03ab701ffffffff02000000000000000000977ae027000000002321032ce5cc649a30eb4f052bc2ff2080c53781ddb0881a4469f77e060c91d32671f5ac00000000";
        auto txHash = "c7578a4909c7000403df354ec8ce4a8a7f9074c935c45491f74783fd1cc03c0e";
        auto tx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
        EXPECT_EQ(tx->getHash(), txHash);
    }
}

TEST_F(BitcoinSynchronizations, MediumP2SHXpubSynchronization) {
    auto const currency = currencies::bitcoin_testnet();

    registerCurrency(currency);

    {
        auto configuration = api::DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", currency.name, configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, P2SH_XPUB_INFO);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiver);

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

TEST_F(BitcoinSynchronizations, SynchronizeP2SHOnceAtATime) {
    auto const currency = currencies::bitcoin_testnet();

    registerCurrency(currency);

    {
        auto configuration = api::DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a62", currency.name, configuration));
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, P2SH_XPUB_INFO);
            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),
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

TEST_F(BitcoinSynchronizations, SynchronizeP2SHFromLastBlock) {
    auto const currency = currencies::bitcoin_testnet();

    registerCurrency(currency);

    {
        auto configuration = api::DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a63", currency.name, configuration));
        createAccount<BitcoinLikeAccount>(wallet, 0, P2SH_XPUB_INFO);
        auto synchronize = [wallet, this] (bool expectNewOp) {
            auto account = wait(wallet->getAccount(0));
            auto numberOfOp = 0;

            auto receiverNumberOp = make_receiver([&numberOfOp](const std::shared_ptr<api::Event> &event) {
                numberOfOp += 1;
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiverNumberOp);
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

TEST_F(BitcoinSynchronizations, EraseDataSinceAfterP2SHSynchronization) {
    auto const currency = currencies::bitcoin_testnet();

    registerCurrency(currency);

    {
        //Set configuration
        auto configuration = api::DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "49'/<coin_type>'/<account>'/<node>/<address>");
        //Create wallet
        auto wallet = wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a63", currency.name, configuration));
        //Create account
        auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2SH_XPUB_INFO);
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
            soci::session sql(services->getDatabaseSessionPool()->getPool());
            BitcoinLikeAccountDatabaseEntry entry;
            auto result = BitcoinLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
            EXPECT_EQ(result, false);
        }

        //Delete wallet
        auto walletCode = wait(walletStore->eraseDataSince(formatedDate));
        EXPECT_EQ(walletCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        //Check if wallet was successfully deleted
        auto walletCount = wait(walletStore->getWalletCount());
        EXPECT_EQ(walletCount, 0);
    }
}
TEST_F(BitcoinSynchronizations, TestNetP2SHSynchronization) {
    auto const currency = currencies::bitcoin_testnet();

    registerCurrency(currency);

    {
        auto configuration = api::DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = wait(walletStore->createWallet("testnet_wallet", currency.name, configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            auto info = wait(wallet->getNextExtendedKeyAccountCreationInfo());
            info.extendedKeys.push_back("tpubDCcvqEHx7prGddpWTfEviiew5YLMrrKy4oJbt14teJZenSi6AYMAs2SNXwYXFzkrNYwECSmobwxESxMCrpfqw4gsUt88bcr8iMrJmbb8P2q");
            EXPECT_EQ(nextIndex, 0);
            auto account = createAccount<BitcoinLikeAccount>(wallet, nextIndex, info);
            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),receiver);
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

TEST_F(BitcoinSynchronizations, DecredParsingAndSerialization) {
    auto const currency = currencies::decred();

    registerCurrency(currency);

    {
        auto wallet = wait(walletStore->createWallet("testnet_wallet", currency.name, api::DynamicObject::newInstance()));
        auto strTx = "01000000016b9b4d4cdd2cf78907e62cddf31911ae4d4af1d89228ae4afc4459edee6a60c40100000000ffffff000240420f000000000000001976a9141d19445f397f6f0d3e2e6d741f61ba66b53886cf88acf0d31d000000000000001976a91415101bac61dca29add75996a0836a469dc8eee0788ac00000000ffffffff01000000000000000000000000ffffffff6a47304402200466bbc2aa8a742e85c3b68911502e73cdcb620ceaaa7a3cd199dbb4f8e9b969022063afeedd37d05e44b655a9de92eb36124acc045baf7b9e2941f81e41af91f1150121030ac79bab351084fdc82b4fa46eaa6a9cd2b5eb97ee93e367422bf47219b54a14";
        auto tx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
    }
}






















