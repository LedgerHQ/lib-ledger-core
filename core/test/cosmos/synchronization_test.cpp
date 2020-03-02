/*
 *
 * cosmos_synchronization.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/06/2019.
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

#include <iostream>
#include <set>

#include "Fixtures.hpp"

#include <gtest/gtest.h>

#include <core/api/Configuration.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/utils/DateUtils.hpp>
#include <core/collections/DynamicObject.hpp>
#include <integration/BaseFixture.hpp>

#include <cosmos/explorers/GaiaCosmosLikeBlockchainExplorer.hpp>
#include <cosmos/CosmosNetworks.hpp>
#include <cosmos/api/CosmosConfigurationDefaults.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <cosmos/CosmosLikeCurrencies.hpp>
#include <cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>
#include <cosmos/CosmosLikeWallet.hpp>
#include <cosmos/CosmosLikeOperationQuery.hpp>
#include <cosmos/CosmosLikeConstants.hpp>

using namespace std;
using namespace ledger::core;
using namespace ledger::testing::cosmos;

api::CosmosLikeNetworkParameters COSMOS = networks::getCosmosLikeNetworkParameters("atom");

class CosmosLikeWalletSynchronization : public BaseFixture {
public:
    void SetUp() override {
        BaseFixture::SetUp();
        backend->enableQueryLogging(true);
    }
};

TEST_F(CosmosLikeWalletSynchronization, GetAccountWithExplorer) {

    auto context = this->dispatcher->getSerialExecutionContext("context");
    auto services = this->newDefaultServices();

    auto explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            services->getDispatcher()->getSerialExecutionContext("explorer"),
            services->getHttpClient(api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT),
            COSMOS,
            std::make_shared<DynamicObject>()
            );

    auto account = ::wait(explorer->getAccount(DEFAULT_ADDRESS));
    ASSERT_EQ(account->address, DEFAULT_ADDRESS);
    ASSERT_EQ(account->accountNumber, "12850");

}

TEST(CosmosLikeBlockchainExplorer, FilterBuilder) {
    auto filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters(
        {GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeyAction,
             cosmos::constants::kEventTypeDelegate),
         GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeySender,
             "cosmostestaddress")});

    ASSERT_STREQ(filter.c_str(), "message.action=delegate&message.sender=cosmostestaddress" );

    filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters({
        GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
            cosmos::constants::kEventTypeTransfer,
            cosmos::constants::kAttributeKeyRecipient,
            "cosmosvalopertestaddress")});


    ASSERT_STREQ(filter.c_str(), "transfer.recipient=cosmosvalopertestaddress");
}



TEST_F(CosmosLikeWalletSynchronization, GetWithdrawDelegationRewardWithExplorer) {
    auto context = this->dispatcher->getSerialExecutionContext("context");
    auto services = this->newDefaultServices();

    auto explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            services->getDispatcher()->getSerialExecutionContext("explorer"),
            services->getHttpClient(api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT),
            COSMOS,
            std::make_shared<DynamicObject>()
    );

    auto filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters({
        GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
            cosmos::constants::kEventTypeTransfer,
            cosmos::constants::kAttributeKeyRecipient,
            DEFAULT_ADDRESS)
    });
    auto transactions = ::wait(explorer->getTransactions(filter, 1, 10));
    ASSERT_TRUE(transactions.size() >= 1) << "At least 1 transaction must be fetched looking at the REST response manually.";
    bool foundTx = false;
    for (const auto& tx : transactions) {
        if (tx->hash == "0DBFC4E8E9E5A64C2C9B5EAAAA0422D99A61CFC5354E15002A061E91200DC2D6") {
            foundTx = true;
            EXPECT_EQ(tx->block->height, 237691);
            EXPECT_EQ(tx->logs.size(), 2);
            size_t withdraw_msg_index = 2;
            if (tx->messages[0].type == cosmos::constants::kMsgWithdrawDelegationReward) {
                withdraw_msg_index = 0;
            } else if (tx->messages[1].type == cosmos::constants::kMsgWithdrawDelegationReward) {
                withdraw_msg_index = 1;
            } else {
                FAIL() << cosmos::constants::kMsgWithdrawDelegationReward << " message not found in tx";
            }
            EXPECT_TRUE(tx->logs[withdraw_msg_index].success);
            EXPECT_EQ(tx->messages[withdraw_msg_index].type, cosmos::constants::kMsgWithdrawDelegationReward);
            const cosmos::MsgWithdrawDelegationReward& msg = boost::get<cosmos::MsgWithdrawDelegationReward>(tx->messages[0].content);
            EXPECT_EQ(msg.delegatorAddress, DEFAULT_ADDRESS);
            EXPECT_EQ(msg.validatorAddress, "cosmosvaloper1sd4tl9aljmmezzudugs7zlaya7pg2895ws8tfs");
            EXPECT_EQ(tx->fee.gas.toInt64(), 200000);
            EXPECT_EQ(tx->fee.amount[withdraw_msg_index].denom, "uatom");
            EXPECT_EQ(tx->fee.amount[withdraw_msg_index].amount, "5000");
            EXPECT_EQ(tx->gasUsed, Option<std::string>("104477"));
            break;
        }
    }
    ASSERT_TRUE(foundTx) << "The transaction we need to test has not been found in the REST request.";
}


TEST_F(CosmosLikeWalletSynchronization, GetErrorTransaction) {
    auto services = this->newDefaultServices();
    auto tx_hash = "4A7823F0F2899AA6EC1DCB2E242C541EDAF90419A3DE03ED885E438FEDB779D4";
    auto validator = "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn";
    auto delegator = "cosmos1k3kg9w60dd5x56vve2s28v3xjp7fp2vn2hjjsa";

    auto explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            services->getDispatcher()->getSerialExecutionContext("explorer"),
            services->getHttpClient(api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT),
            COSMOS,
            std::make_shared<DynamicObject>()
    );
    auto tx = ::wait(explorer->getTransactionByHash(tx_hash));
    ASSERT_EQ(tx->hash, tx_hash);
    EXPECT_EQ(tx->block->height, 768780);
    EXPECT_EQ(tx->logs.size(), 1);
    EXPECT_FALSE(tx->logs[0].success);
    EXPECT_EQ(tx->logs[0].log, "{\"codespace\":\"sdk\",\"code\":10,\"message\":\"insufficient account funds; 2412592uatom < 2417501uatom\"}");
    EXPECT_EQ(tx->messages[0].type, cosmos::constants::kMsgDelegate);
    const cosmos::MsgDelegate& msg = boost::get<cosmos::MsgDelegate>(tx->messages[0].content);
    EXPECT_EQ(msg.delegatorAddress, delegator);
    EXPECT_EQ(msg.validatorAddress, validator);
    EXPECT_EQ(msg.amount.amount, "2417501");
    EXPECT_EQ(msg.amount.denom, "uatom");
    EXPECT_EQ(tx->fee.gas.toInt64(), 200000);
    EXPECT_EQ(tx->fee.amount[0].denom, "uatom");
    EXPECT_EQ(tx->fee.amount[0].amount, "5000");
    EXPECT_EQ(tx->gasUsed, Option<std::string>("47235"));
}


TEST_F(CosmosLikeWalletSynchronization, GetSendWithExplorer) {
    auto services = this->newDefaultServices();
    auto tx_hash = "F4B8CB550B498F744CCC420907B80D0B068250972F975354A873CD1CCF9B000A";
    auto receiver = "cosmos1xxkueklal9vejv9unqu80w9vptyepfa95pd53u";
    // Note : the sender of the message is also the sender of the funds in this transaction.
    auto sender = "cosmos15v50ymp6n5dn73erkqtmq0u8adpl8d3ujv2e74";

    auto explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            services->getDispatcher()->getSerialExecutionContext("explorer"),
            services->getHttpClient(api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT),
            COSMOS,
            std::make_shared<DynamicObject>()
    );
    auto tx = ::wait(explorer->getTransactionByHash(tx_hash));
    ASSERT_EQ(tx->hash, tx_hash);
    EXPECT_EQ(tx->block->height, 453223);
    EXPECT_EQ(tx->logs.size(), 1);
    EXPECT_TRUE(tx->logs[0].success);
    EXPECT_EQ(tx->messages[0].type, cosmos::constants::kMsgSend);
    const cosmos::MsgSend& msg = boost::get<cosmos::MsgSend>(tx->messages[0].content);
    EXPECT_EQ(msg.fromAddress, sender);
    EXPECT_EQ(msg.toAddress, receiver);
    EXPECT_EQ(msg.amount[0].amount, "270208360");
    EXPECT_EQ(msg.amount[0].denom, "uatom");
    EXPECT_EQ(tx->fee.gas.toInt64(), 200000);
    EXPECT_EQ(tx->fee.amount[0].denom, "uatom");
    EXPECT_EQ(tx->fee.amount[0].amount, "30");
    EXPECT_EQ(tx->gasUsed, Option<std::string>("41014"));
}

TEST_F(CosmosLikeWalletSynchronization, GetDelegateWithExplorer) {
    auto services = this->newDefaultServices();
    auto delegator = "cosmos1ytpz9gt59hssp5m5sknuzrwse88glqhgcrypxj";
    auto validator = "cosmosvaloper1ey69r37gfxvxg62sh4r0ktpuc46pzjrm873ae8";

    auto explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            services->getDispatcher()->getSerialExecutionContext("explorer"),
            services->getHttpClient(api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT),
            COSMOS,
            std::make_shared<DynamicObject>()
    );
    auto filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters(
        {GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeyAction,
             cosmos::constants::kEventTypeDelegate),
         GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeySender,
             delegator)});
    auto transactions = ::wait(explorer->getTransactions(filter, 1, 10));
    ASSERT_TRUE(transactions.size() >= 1);
    bool foundTx = false;
    for (const auto& tx : transactions) {
        if (tx->hash == "BD77DF6A76066AA79DAA7705B9F0DC6B66B7E6FBB3D1FD28A07D6A0EED7AE6B5") {
            foundTx = true;
            EXPECT_EQ(tx->block->height, 660081);
            EXPECT_EQ(tx->logs.size(), 1);
            EXPECT_TRUE(tx->logs[0].success);
            EXPECT_EQ(tx->messages[0].type, cosmos::constants::kMsgDelegate);
            const cosmos::MsgDelegate& msg = boost::get<cosmos::MsgDelegate>(tx->messages[0].content);
            EXPECT_EQ(msg.delegatorAddress, delegator);
            EXPECT_EQ(msg.validatorAddress, validator);
            EXPECT_EQ(msg.amount.amount, "25257508");
            EXPECT_EQ(msg.amount.denom, "uatom");
            EXPECT_EQ(tx->fee.gas.toInt64(), 300000);
            EXPECT_EQ(tx->fee.amount[0].denom, "uatom");
            EXPECT_EQ(tx->fee.amount[0].amount, "2500");
            EXPECT_EQ(tx->gasUsed, Option<std::string>("104477"));
            break;
        }
    }
    ASSERT_TRUE(foundTx);
}

TEST_F(CosmosLikeWalletSynchronization, GetCurrentBlockWithExplorer) {
    std::string address = "cosmos16xkkyj97z7r83sx45xwk9uwq0mj0zszlf6c6mq";
    auto context = this->dispatcher->getSerialExecutionContext("context");
    auto services = this->newDefaultServices();

    auto explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            services->getDispatcher()->getSerialExecutionContext("explorer"),
            services->getHttpClient(api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT),
            COSMOS,
            std::make_shared<DynamicObject>()
    );

    auto block = ::wait(explorer->getCurrentBlock());
    EXPECT_TRUE(block->blockHash.size() > 0);
    EXPECT_TRUE(block->height > 0);
}

TEST_F(CosmosLikeWalletSynchronization, MediumXpubSynchronization) {
    auto walletName = "e847815f-488a-4301-b67c-378a5e9c8a61";
    auto services = newDefaultServices();
    auto walletStore = newWalletStore(services);
    wait(walletStore->addCurrency(currencies::ATOM));

    auto factory = std::make_shared<CosmosLikeWalletFactory>(currencies::ATOM, services);
    walletStore->registerFactory(currencies::ATOM, factory);
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = std::dynamic_pointer_cast<CosmosLikeWallet>(
                wait(walletStore->createWallet(walletName, currencies::ATOM.name, configuration)));
        std::set<std::string> emittedOperations;
        {
            auto accountInfo = wait(wallet->getNextAccountCreationInfo());
            EXPECT_EQ(accountInfo.index, 0);
            accountInfo.publicKeys.push_back(hex::toByteArray(DEFAULT_HEX_PUB_KEY));
                        auto account = ledger::testing::cosmos::createCosmosLikeAccount(
                                wallet, accountInfo.index, accountInfo);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });
            auto address = wait(account->getFreshPublicAddresses())[0]->toString();
            EXPECT_EQ(address, DEFAULT_ADDRESS);
            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = wait(account->getBalance());
                fmt::print("Balance: {} uatom\n", balance->toString());
                auto txBuilder = std::dynamic_pointer_cast<CosmosLikeTransactionBuilder>(account->buildTransaction());
                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            account->synchronize()->subscribe(
                dispatcher->getMainExecutionContext(), receiver);

            dispatcher->waitUntilStopped();

            auto block = wait(account->getLastBlock());
            fmt::print("Block height: {}\n", block.height);
            auto ops = wait(std::dynamic_pointer_cast<CosmosLikeOperationQuery>(account->queryOperations()->complete())->execute());
            fmt::print("Ops: {}\n", ops.size());
        }
    }
}
