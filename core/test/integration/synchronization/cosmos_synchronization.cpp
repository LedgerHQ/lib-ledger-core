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
#include <numeric>

#include <test/cosmos/Fixtures.hpp>
#include "../BaseFixture.h"

#include <gtest/gtest.h>

#include <api/Configuration.hpp>
#include <api/KeychainEngines.hpp>
#include <api/PoolConfiguration.hpp>
#include <utils/DateUtils.hpp>
#include <utils/hex.h>
#include <collections/DynamicObject.hpp>
#include <math/BigInt.h>

#include <wallet/cosmos/explorers/GaiaCosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>
#include <api/CosmosConfigurationDefaults.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>

#include <wallet/cosmos/database/CosmosLikeOperationDatabaseHelper.hpp>

using namespace std;
using namespace ledger::core;
using namespace ledger::testing::cosmos;

api::CosmosLikeNetworkParameters COSMOS_PARAMS = networks::getCosmosLikeNetworkParameters("atom");

class CosmosLikeWalletSynchronization : public BaseFixture {
public:
    void SetUp() override {
        BaseFixture::SetUp();
        auto worker = dispatcher->getSerialExecutionContext("worker");
        auto client = std::make_shared<HttpClient>(
            api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT, http, worker);

#ifdef PG_SUPPORT
    const bool usePostgreSQL = true;
    auto poolConfig = DynamicObject::newInstance();
    poolConfig->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
    pool = newDefaultPool("postgres", "", poolConfig, usePostgreSQL);
#else
    pool = newDefaultPool();
#endif

        explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            worker,
            client,
            COSMOS_PARAMS,
            std::make_shared<DynamicObject>());
    }

    void setupTest(std::shared_ptr<CosmosLikeAccount>& account,
                   std::shared_ptr<AbstractWallet>& wallet,
                   const std::string& pubKey) {

        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>");
        wallet = uv::wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "cosmos", configuration));

        auto accountInfo = uv::wait(wallet->getNextAccountCreationInfo());
        EXPECT_EQ(accountInfo.index, 0);
        accountInfo.publicKeys.push_back(hex::toByteArray(pubKey));

        account = ledger::testing::cosmos::createCosmosLikeAccount(wallet, accountInfo.index, accountInfo);
    }

    void performSynchro(const std::shared_ptr<CosmosLikeAccount>& account) {
        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
            fmt::print("Received event {}\n", api::to_string(event->getCode()));
            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) {
                return;
            }
            EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

            auto balance = uv::wait(account->getBalance());
            fmt::print("Balance: {} uatom\n", balance->toString());

            auto block = uv::wait(account->getLastBlock());
            fmt::print("Block height: {}\n", block.height);
            EXPECT_GT(block.height, 0);

            getTestExecutionContext()->stop();
        });

        auto bus = account->synchronize();
        bus->subscribe(getTestExecutionContext(), receiver);
        getTestExecutionContext()->waitUntilStopped();
     }

    void TearDown() override {
        uv::wait(pool->freshResetAll());
        BaseFixture::TearDown();
    }

    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<GaiaCosmosLikeBlockchainExplorer> explorer;
};

TEST_F(CosmosLikeWalletSynchronization, GetAccountWithExplorer) {

    auto account = uv::wait(explorer->getAccount(DEFAULT_ADDRESS));
    EXPECT_EQ(account->address, DEFAULT_ADDRESS);
    EXPECT_EQ(account->accountNumber, "12850");
    EXPECT_EQ(account->withdrawAddress, DEFAULT_ADDRESS)
        << "Withdraw address has not been modified on this address";
}


TEST_F(CosmosLikeWalletSynchronization, InternalFeesMessageInTransaction) {
    /// This transaction contains 2 messages. One internal message to store fees
    /// is added after the transaction is retrieved from the network, hence the
    /// transaction should contain 3 messages, the last one being the one added
    /// specifically for the fees.
    const auto transaction = uv::wait(explorer->getTransactionByHash("0DBFC4E8E9E5A64C2C9B5EAAAA0422D99A61CFC5354E15002A061E91200DC2D6"));

    ASSERT_NE(transaction, nullptr);
    EXPECT_EQ(transaction->messages.size(), 3);
    EXPECT_EQ(transaction->logs.size(), 3);
    const auto feeMessageIndex = transaction->messages.size() - 1;
    const auto& feeMessage = transaction->messages[feeMessageIndex];
    const auto& feeMessageLog = transaction->logs[feeMessageIndex];

    auto reduceAmounts = [](const std::vector<cosmos::Coin>& coins) {
        return std::accumulate(std::begin(coins), std::end(coins), BigInt::ZERO,
                [](BigInt s, const cosmos::Coin& coin) {
                    return s + BigInt::fromString(coin.amount);
                });
    };

    // check the internal fees message content
    ASSERT_EQ(feeMessage.type, cosmos::constants::kMsgFees);
    const auto& feeMessageContent = boost::get<cosmos::MsgFees>(feeMessage.content);
    EXPECT_EQ(reduceAmounts(transaction->fee.amount), BigInt::fromString(feeMessageContent.fees.amount));
    EXPECT_EQ(feeMessageContent.payerAddress, DEFAULT_ADDRESS);

    // check the logs for the fees message
    EXPECT_EQ(feeMessageLog.messageIndex, feeMessageIndex);
    EXPECT_EQ(feeMessageLog.success, true);
    EXPECT_EQ(feeMessageLog.log, "");
}


TEST_F(CosmosLikeWalletSynchronization, GetWithdrawDelegationRewardWithExplorer) {

    auto filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters({
        GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
            cosmos::constants::kEventTypeTransfer,
            cosmos::constants::kAttributeKeyRecipient,
            DEFAULT_ADDRESS)
    });
    auto bulk = uv::wait(explorer->getTransactions(filter, 1, 10));
    auto transactions = bulk->transactions;
    ASSERT_TRUE(transactions.size() >= 1) << "At least 1 transaction must be fetched looking at the REST response manually.";
    bool foundTx = false;
    for (const auto& tx : transactions) {
        if (tx.hash == "0DBFC4E8E9E5A64C2C9B5EAAAA0422D99A61CFC5354E15002A061E91200DC2D6") {
            foundTx = true;
            EXPECT_EQ(tx.block->height, 237691);
            EXPECT_EQ(tx.logs.size(), 3);
            size_t withdraw_msg_index = 2;
            if (tx.messages[0].type == cosmos::constants::kMsgWithdrawDelegationReward) {
                withdraw_msg_index = 0;
            } else if (tx.messages[1].type == cosmos::constants::kMsgWithdrawDelegationReward) {
                withdraw_msg_index = 1;
            } else {
                FAIL() << cosmos::constants::kMsgWithdrawDelegationReward << " message not found in tx";
            }
            EXPECT_TRUE(tx.logs[withdraw_msg_index].success);
            EXPECT_TRUE(tx.messages[withdraw_msg_index].log.success);
            EXPECT_EQ(tx.messages[withdraw_msg_index].type, cosmos::constants::kMsgWithdrawDelegationReward);
            const cosmos::MsgWithdrawDelegationReward& msg = boost::get<cosmos::MsgWithdrawDelegationReward>(tx.messages[0].content);
            EXPECT_EQ(msg.delegatorAddress, DEFAULT_ADDRESS);
            EXPECT_EQ(msg.validatorAddress, "cosmosvaloper1sd4tl9aljmmezzudugs7zlaya7pg2895ws8tfs");
            EXPECT_EQ(tx.fee.gas.toInt64(), 200000);
            EXPECT_EQ(tx.fee.amount[withdraw_msg_index].denom, "uatom");
            EXPECT_EQ(tx.fee.amount[withdraw_msg_index].amount, "5000");
            EXPECT_EQ(tx.gasUsed, Option<std::string>("104477"));
            break;
        }
    }
    ASSERT_TRUE(foundTx) << "The transaction we need to test has not been found in the REST request.";
}


TEST_F(CosmosLikeWalletSynchronization, GetErrorTransaction) {
    auto tx_hash = "4A7823F0F2899AA6EC1DCB2E242C541EDAF90419A3DE03ED885E438FEDB779D4";
    auto validator = "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn";
    auto delegator = "cosmos1k3kg9w60dd5x56vve2s28v3xjp7fp2vn2hjjsa";

    auto tx = uv::wait(explorer->getTransactionByHash(tx_hash));
    ASSERT_EQ(tx->hash, tx_hash);
    EXPECT_EQ(tx->block->height, 768780);
    EXPECT_EQ(tx->logs.size(), 2);
    // Logs are stored twice now, but only in serialized types
    // DB will only ever store logs in cosmos_messages table anyway
    EXPECT_FALSE(tx->logs[0].success);
    EXPECT_EQ(tx->logs[0].log, "{\"codespace\":\"sdk\",\"code\":10,\"message\":\"insufficient account funds; 2412592uatom < 2417501uatom\"}");
    EXPECT_FALSE(tx->messages[0].log.success);
    EXPECT_EQ(tx->messages[0].log.log, R"esc({"codespace":"sdk","code":10,"message":"insufficient account funds; 2412592uatom < 2417501uatom"})esc");
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
    auto tx_hash = "F4B8CB550B498F744CCC420907B80D0B068250972F975354A873CD1CCF9B000A";
    auto receiver = "cosmos1xxkueklal9vejv9unqu80w9vptyepfa95pd53u";
    // Note : the sender of the message is also the sender of the funds in this transaction.
    auto sender = "cosmos15v50ymp6n5dn73erkqtmq0u8adpl8d3ujv2e74";

    auto tx = uv::wait(explorer->getTransactionByHash(tx_hash));
    ASSERT_EQ(tx->hash, tx_hash);
    EXPECT_EQ(tx->block->height, 453223);
    EXPECT_EQ(tx->logs.size(), 2);
    EXPECT_TRUE(tx->logs[0].success);
    EXPECT_TRUE(tx->messages[0].log.success);
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
    auto delegator = "cosmos1ytpz9gt59hssp5m5sknuzrwse88glqhgcrypxj";
    auto validator = "cosmosvaloper1ey69r37gfxvxg62sh4r0ktpuc46pzjrm873ae8";

    auto filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters(
        {GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeyAction,
             cosmos::constants::kEventTypeDelegate),
         GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeySender,
             delegator)});
    auto bulk = uv::wait(explorer->getTransactions(filter, 1, 10));
    auto transactions = bulk->transactions;
    ASSERT_TRUE(transactions.size() >= 1);
    bool foundTx = false;
    for (const auto& tx : transactions) {
        if (tx.hash == "BD77DF6A76066AA79DAA7705B9F0DC6B66B7E6FBB3D1FD28A07D6A0EED7AE6B5") {
            foundTx = true;
            EXPECT_EQ(tx.block->height, 660081);
            EXPECT_EQ(tx.logs.size(), 2);
            EXPECT_TRUE(tx.logs[0].success);
            EXPECT_TRUE(tx.messages[0].log.success);
            EXPECT_EQ(tx.messages[0].type, cosmos::constants::kMsgDelegate);
            const cosmos::MsgDelegate& msg = boost::get<cosmos::MsgDelegate>(tx.messages[0].content);
            EXPECT_EQ(msg.delegatorAddress, delegator);
            EXPECT_EQ(msg.validatorAddress, validator);
            EXPECT_EQ(msg.amount.amount, "25257508");
            EXPECT_EQ(msg.amount.denom, "uatom");
            EXPECT_EQ(tx.fee.gas.toInt64(), 300000);
            EXPECT_EQ(tx.fee.amount[0].denom, "uatom");
            EXPECT_EQ(tx.fee.amount[0].amount, "2500");
            EXPECT_EQ(tx.gasUsed, Option<std::string>("104477"));
            break;
        }
    }
    ASSERT_TRUE(foundTx);
}

TEST_F(CosmosLikeWalletSynchronization, GetCurrentBlockWithExplorer) {
    std::string address = "cosmos16xkkyj97z7r83sx45xwk9uwq0mj0zszlf6c6mq";

    auto block = uv::wait(explorer->getCurrentBlock());
    EXPECT_TRUE(block->hash.size() > 0);
    EXPECT_TRUE(block->height > 0);
}

TEST_F(CosmosLikeWalletSynchronization, DISABLED_MediumXpubSynchronization) {
    auto walletName = "8d99cc44-9061-43a4-9edd-f938d2007926";
#ifdef PG_SUPPORT
    const bool usePostgreSQL = true;
    auto poolConfig = DynamicObject::newInstance();
    poolConfig->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
    auto pool = newDefaultPool("postgres", "", poolConfig, usePostgreSQL);
#else
    auto pool = newDefaultPool();
#endif
    backend->enableQueryLogging(true);

    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = uv::wait(pool->createWallet(walletName, currencies::ATOM.name, configuration));
        std::set<std::string> emittedOperations;
        {
            auto accountInfo = uv::wait(wallet->getNextAccountCreationInfo());
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
            auto address = uv::wait(account->getFreshPublicAddresses())[0]->toString();
            EXPECT_EQ(address, DEFAULT_ADDRESS);
            auto eventBus = pool->getEventBus();
            eventBus->subscribe(getTestExecutionContext(), receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = uv::wait(account->getBalance());
                fmt::print("Balance: {} uatom\n", balance->toString());
                auto txBuilder = std::dynamic_pointer_cast<CosmosLikeTransactionBuilder>(account->buildTransaction());
                getTestExecutionContext()->stop();
            });

            auto restoreKey = account->getRestoreKey();
            auto bus = account->synchronize();
            bus->subscribe(getTestExecutionContext(), receiver);

            getTestExecutionContext()->waitUntilStopped();

            auto block = uv::wait(account->getLastBlock());
            fmt::print("Block height: {}\n", block.height);
            EXPECT_GT(block.height, 0);

            auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            fmt::print("Ops: {}\n", ops.size());
            ASSERT_GT(ops.size(), 0);
            ASSERT_TRUE(ops[0]->getBlockHeight())
                << "The first operation should have a block height.";
            EXPECT_GT(ops[0]->getBlockHeight().value(), 0)
                << "The first operation should have a non 0 height.";
            EXPECT_LT(ops[0]->getBlockHeight().value(), block.height)
                << "The first operation should not have the same height as the last block.";

            const auto sequenceNumber = account->getInfo().sequence;
            const int sequence = std::atoi(sequenceNumber.c_str());
            EXPECT_GE(sequence, 1226) << "Sequence was at 1226 on 2020-03-26";

            const auto accountNumber = account->getInfo().accountNumber;
            EXPECT_EQ(accountNumber, "12850") << "Account number is a network constant for a given address";
        }
    }
}

TEST_F(CosmosLikeWalletSynchronization, Balances)
{
    std::string hexPubKey =
        "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a";  // Obelix

    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<CosmosLikeAccount> account;

    setupTest(account, wallet, hexPubKey);

    const std::string address = account->getKeychain()->getAddress()->toBech32();
    const std::string mintscanExplorer = fmt::format("https://www.mintscan.io/account/{}", address);

    const auto totalBalance = uv::wait(account->getTotalBalance())->toLong();
    const auto delegatedBalance = uv::wait(account->getDelegatedBalance())->toLong();
    const auto pendingRewards = uv::wait(account->getPendingRewardsBalance())->toLong();
    const auto unbondingBalance = uv::wait(account->getUnbondingBalance())->toLong();
    const auto spendableBalance = uv::wait(account->getSpendableBalance())->toLong();

    EXPECT_LE(delegatedBalance, totalBalance)
        << "Delegated Coins fall under Total Balance, so delegatedBalance <= totalBalance";
    // Strict comparison here because
    // it's impossible to have pending rewards without
    // at least some delegatedBalance or unbonding balance
    EXPECT_LT(pendingRewards, totalBalance)
        << "Pending rewards fall under Total Balance, so pendingRewards < totalBalance";
    EXPECT_LE(unbondingBalance, totalBalance)
        << "Unbondings fall under Total Balance, so unbondingBalance <= totalBalance";
    EXPECT_LE(spendableBalance, totalBalance)
        << "Spendable Coins fall under Total Balance, so spendableBalance <= totalBalance";

    EXPECT_GE(pendingRewards, 1) << fmt::format(
        "Check {}  to see if the account really has <1 uatom of pending rewards", mintscanExplorer);
    EXPECT_GE(totalBalance, 1000000) << fmt::format(
        "Check {}  to see if the account really has <1 ATOM of total balance", mintscanExplorer);
    EXPECT_GE(delegatedBalance, 2800) << fmt::format(
        "Check {}  to see if the account really has <0.002800 ATOM of total delegations",
        mintscanExplorer);
    EXPECT_GE(spendableBalance, 1000) << fmt::format(
        "Check {}  to see if the account really has <0.001 ATOM of available (spendable) balance",
        mintscanExplorer);
    // Unbondings are moving too much to assert the amount.
}

TEST_F(CosmosLikeWalletSynchronization, DISABLED_AllTransactionsSynchronization) {
    // FIXME Use an account that has all expected types of transactions
    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    performSynchro(account);

    auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    fmt::print("Ops: {}\n", ops.size());
    EXPECT_GT(ops.size(), 0);

    auto foundMsgSend = false;
    auto foundMsgDelegate = false;
    auto foundMsgBeginRedelegate = false;
    auto foundMsgUndelegate = false;
    auto foundMsgSubmitProposal = false;
    auto foundMsgVote = false;
    auto foundMsgDeposit = false;
    auto foundMsgWithdrawDelegationReward = false;
    auto foundMsgMultiSend = false;
    auto foundMsgCreateValidator = false;
    auto foundMsgEditValidator = false;
    auto foundMsgSetWithdrawAddress = false;
    auto foundMsgWithdrawDelegatorReward = false;
    auto foundMsgWithdrawValidatorCommission = false;
    auto foundMsgUnjail = false;

    for (auto op : ops) {
        auto cosmosOp = op->asCosmosLikeOperation();

        std::cout << "Found operation type: " << cosmosOp->getMessage()->getRawMessageType() << std::endl;

        switch (cosmosOp->getMessage()->getMessageType()) {
            case api::CosmosLikeMsgType::MSGSEND: foundMsgSend = true; break;
            case api::CosmosLikeMsgType::MSGDELEGATE: foundMsgDelegate = true; break;
            case api::CosmosLikeMsgType::MSGBEGINREDELEGATE: foundMsgBeginRedelegate = true; break;
            case api::CosmosLikeMsgType::MSGUNDELEGATE: foundMsgUndelegate = true; break;
            case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL: foundMsgSubmitProposal = true; break;
            case api::CosmosLikeMsgType::MSGVOTE: foundMsgVote = true; break;
            case api::CosmosLikeMsgType::MSGDEPOSIT: foundMsgDeposit = true; break;
            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD: foundMsgWithdrawDelegationReward = true; break;
            case api::CosmosLikeMsgType::MSGMULTISEND: foundMsgMultiSend = true; break;
            case api::CosmosLikeMsgType::MSGCREATEVALIDATOR: foundMsgCreateValidator = true; break;
            case api::CosmosLikeMsgType::MSGEDITVALIDATOR: foundMsgEditValidator = true; break;
            case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS: foundMsgSetWithdrawAddress = true; break;
            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD: foundMsgWithdrawDelegatorReward = true; break;
            case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION: foundMsgWithdrawValidatorCommission = true; break;
            case api::CosmosLikeMsgType::MSGUNJAIL: foundMsgUnjail = true; break;
            case api::CosmosLikeMsgType::UNSUPPORTED: break;
        }
    }

    EXPECT_TRUE(foundMsgSend);
    EXPECT_TRUE(foundMsgDelegate);
    EXPECT_TRUE(foundMsgBeginRedelegate);
    EXPECT_TRUE(foundMsgUndelegate);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgSubmitProposal);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgVote);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgDeposit);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgWithdrawDelegationReward);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgMultiSend);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgCreateValidator);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgEditValidator);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgSetWithdrawAddress);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgWithdrawDelegatorReward);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgWithdrawValidatorCommission);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgUnjail);
}

TEST_F(CosmosLikeWalletSynchronization, ValidatorSet) {
    // This test assumes that HuobiPool and BinanceStaking are always in the validator set
    const auto huobi_pool_address = "cosmosvaloper1kn3wugetjuy4zetlq6wadchfhvu3x740ae6z6x";
    bool foundHuobi = false;
    const auto binance_staking_pub_address = "cosmosvalconspub1zcjduepqtw8862dhw8uty58d6t2szfd6kqram2t234zjteaaeem6l45wclaq8l60gn";
    bool foundBinance = false;

    auto set = uv::wait(explorer->getActiveValidatorSet());

    EXPECT_EQ(set.size(), 125) << "currently cosmoshub-3 has 125 active validators";

    for (const auto & validator : set) {
        if (validator.consensusPubkey == binance_staking_pub_address) {
            foundBinance = true;
        } else if (validator.operatorAddress == huobi_pool_address){
            foundHuobi = true;
        } else {
            continue;
        }
    }

    EXPECT_TRUE(foundHuobi) << "Huobi Pool is expected to always be in the validator set";
    EXPECT_TRUE(foundBinance) << "Binance Staking is expected to always be in the validator set";
}

TEST_F(CosmosLikeWalletSynchronization, DISABLED_ValidatorInfo) {
    // This test assumes that HuobiPool and BinanceStaking are always in the validator set
    const auto bisonTrailsAddress = "cosmosvaloper1uxh465053nq3at4dn0jywgwq3s9sme3la3drx6";
    const auto bisonTrailsValConsPubAddress = "cosmosvalconspub1zcjduepqc5y2du793cjut0cn6v7thp3xlvphggk6rt2dhw9ekjla5wtkm7nstmv5vy";
    const auto mintscanAddress = fmt::format("https://www.mintscan.io/validators/{}", bisonTrailsAddress);


    auto valInfo = uv::wait(explorer->getValidatorInfo(bisonTrailsAddress));
    ASSERT_EQ(valInfo.operatorAddress, bisonTrailsAddress) << "We should fetch the expected validator";
    ASSERT_EQ(valInfo.consensusPubkey, bisonTrailsValConsPubAddress) << "We should fetch the expected validator";


    EXPECT_EQ(valInfo.validatorDetails.moniker, "Bison Trails");
    ASSERT_TRUE(valInfo.validatorDetails.identity);
    EXPECT_EQ(valInfo.validatorDetails.identity.value(), "A296556FF603197C");
    ASSERT_TRUE(valInfo.validatorDetails.website);
    EXPECT_EQ(valInfo.validatorDetails.website.value(), "https://bisontrails.co");
    ASSERT_TRUE(valInfo.validatorDetails.details);
    EXPECT_EQ(valInfo.validatorDetails.details.value(), "Bison Trails is the easiest way to run infrastructure on multiple blockchains.");

    EXPECT_EQ(valInfo.commission.rates.maxRate, "0.500000000000000000");
    EXPECT_EQ(valInfo.commission.rates.maxChangeRate, "0.010000000000000000");
    EXPECT_GE(valInfo.commission.updateTime, DateUtils::fromJSON("2019-03-13T23:00:00Z")) <<
        "As of this test writing, last update was on 2019-03-13T23:00:00Z. So updateTime should be at least as recent as this timestamp.";
    // Specify locale for std::stof -- Also, why we just pass strings instead of parsing as much as possible
    setlocale(LC_NUMERIC, "C");
    EXPECT_LE(std::stof(valInfo.commission.rates.rate), std::stof(valInfo.commission.rates.maxRate));

    EXPECT_EQ(valInfo.unbondingHeight, 0) <<
        fmt::format("Expecting BisonTrails to never have been jailed. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_FALSE(valInfo.unbondingTime) <<
        fmt::format("Expecting BisonTrails to never have been jailed. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_EQ(valInfo.minSelfDelegation, "1") <<
        fmt::format("Expecting BisonTrails to have '1' minimum self delegation. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_FALSE(valInfo.jailed) <<
        fmt::format("Expecting BisonTrails to never have been jailed. Check {} to see if the assertion holds", mintscanAddress);
    //EXPECT_GE(BigInt::fromString(valInfo.votingPower).toUint64(), BigInt::fromString("400000000000").toUint64()) <<
    //    fmt::format("Expecting BisonTrails voting power to be > 400_000 ATOM. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_EQ(valInfo.activeStatus, 2) <<
        fmt::format("Expecting BisonTrails to be active (and that currently the explorer returns 2 for this status). Check {} to see if the assertion holds", mintscanAddress);

    EXPECT_FALSE(valInfo.distInfo.validatorCommission.empty()) << "Bison Trails should have accumulated at least *some* validator commission";
    EXPECT_FALSE(valInfo.distInfo.selfBondRewards.empty()) << "Bison Trails should have accumulated at least *some* self delegation rewards";

    EXPECT_FALSE(valInfo.signInfo.tombstoned) << "Bison Trails is not expected to be tombstoned";
    EXPECT_GE(valInfo.signInfo.missedBlocksCounter, 0) << "This value cannot be negative";
    EXPECT_GE(valInfo.signInfo.jailedUntil, DateUtils::fromJSON("1970-01-01T00:00:00Z")) << "This value cannot be before epoch";
}

TEST_F(CosmosLikeWalletSynchronization, DISABLED_BalanceHistoryOperationQuery) {

    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    performSynchro(account);

    const auto &uid = account->getAccountUid();
    soci::session sql(wallet->getDatabase()->getPool());
    std::vector<Operation> operations;

    auto keychain = account->getKeychain();
    std::function<bool(const std::string &)> filter = [&keychain](const std::string addr) -> bool {
        return keychain->contains(addr);
    };

    //Get operations related to an account
    CosmosLikeOperationDatabaseHelper::queryOperations(
        sql,
        uid,
        operations,
        filter);

    ASSERT_GE(operations.size(), 17) << "As of 2020-03-19, there are 17 operations picked up by the query";
}

TEST_F(CosmosLikeWalletSynchronization, GetAccountDelegations) {

    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    auto delegations = uv::wait(account->getDelegations());
    EXPECT_GE(delegations.size(), 2);

    BigInt delegatedAmount;
    for (auto& delegation : delegations) {
        delegatedAmount = delegatedAmount + *(std::dynamic_pointer_cast<ledger::core::Amount>(delegation->getDelegatedAmount())->value());
    }
    EXPECT_GE(delegatedAmount.toUint64(), 1000000UL); // 1 ATOM

}

TEST_F(CosmosLikeWalletSynchronization, GetAccountPendingRewards) {

    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    auto rewards = uv::wait(account->getPendingRewards());
    EXPECT_GE(rewards.size(), 2);

    BigInt pendingReward;
    for (auto& reward : rewards) {
        pendingReward = pendingReward + *(std::dynamic_pointer_cast<ledger::core::Amount>(reward->getRewardAmount())->value());
    }
    EXPECT_GE(pendingReward.toUint64(), 1000UL); // 1000 uATOM

}

namespace {

void GenericGasLimitEstimationTest(const std::string& strTx, CosmosLikeWalletSynchronization& t)
{
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawSignedTransaction(ledger::core::currencies::ATOM, strTx);
    const auto estimatedGasLimit = uv::wait(t.explorer->getEstimatedGasLimit(tx));
    EXPECT_GE(estimatedGasLimit->toUint64(), 0);
}

} // namespace

TEST_F(CosmosLikeWalletSynchronization, GasLimitEstimationForTransfer) {
    const auto strTx = "{\"account_number\":\"6571\","
        "\"chain_id\":\"cosmoshub-3\","
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msgs\":["
        "{\"type\":\"cosmos-sdk/MsgSend\","
        "\"value\":{"
        "\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
        "\"from_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
        "\"to_address\":\"cosmos16xyempempp92x9hyzz9wrgf94r6j9h5f06pxxv\""
        "}}],"
        "\"sequence\":\"0\"}";
    GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosLikeWalletSynchronization, GasLimitEstimationForWithdrawingRewards) {
    const auto strTx = "{\"account_number\":\"6571\","
        "\"chain_id\":\"cosmoshub-3\","
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msgs\":["
        "{\"type\":\"cosmos-sdk/MsgWithdrawDelegationReward\""
        ",\"value\":{"
        "\"delegator_address\":\"cosmos167w96tdvmazakdwkw2u57227eduula2cy572lf\","
        "\"validator_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}],"
        "\"sequence\":\"0\"}";
    GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosLikeWalletSynchronization, GasLimitEstimationForDelegation) {
    const auto strTx = "{\"account_number\":\"6571\","
        "\"chain_id\":\"cosmoshub-3\","
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msgs\":["
        "{\"type\":\"cosmos-sdk/MsgDelegate\","
        "\"value\":{"
        "\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
        "\"delegator_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
        "\"validator_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}],"
        "\"sequence\":\"0\"}";
    GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosLikeWalletSynchronization, GasLimitEstimationForUnDelegation) {
    const auto strTx = "{\"account_number\":\"6571\","
        "\"chain_id\":\"cosmoshub-3\","
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msgs\":["
        "{\"type\":\"cosmos-sdk/MsgUndelegate\","
        "\"value\":{"
        "\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
        "\"delegator_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
        "\"validator_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}],"
        "\"sequence\":\"0\"}";
    GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosLikeWalletSynchronization, GasLimitEstimationForRedelegation) {
    const auto strTx = "{\"account_number\":\"6571\","
        "\"chain_id\":\"cosmoshub-3\","
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msgs\":["
        "{\"type\":\"cosmos-sdk/MsgBeginRedelegate\","
        "\"value\":{\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
        "\"delegator_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
        "\"validator_dst_address\":\"cosmosvaloper1sd4tl9aljmmezzudugs7zlaya7pg2895ws8tfs\","
        "\"validator_src_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}],"
        "\"sequence\":\"0\"}";
    GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosLikeWalletSynchronization, PendingUnbondings) {
    std::string hexPubKey =
        "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a";  // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);
    const std::string address = account->getKeychain()->getAddress()->toBech32();
    const std::string mintscanExplorer = fmt::format("https://www.mintscan.io/account/{}", address);

    // First synchro
    performSynchro(account);

    auto unbondings = uv::wait(account->getUnbondings());
    EXPECT_GE(unbondings.size(), 1) << fmt::format(
        "Expecting at least 1 unbonding here for Obelix (explorer link : {}).", mintscanExplorer);
}

TEST_F(CosmosLikeWalletSynchronization, PendingRedelegations) {
    std::string hexPubKey =
        "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a";  // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);
    const std::string address = account->getKeychain()->getAddress()->toBech32();
    const std::string mintscanExplorer = fmt::format("https://www.mintscan.io/account/{}", address);

    // First synchro
    performSynchro(account);

    auto redelegations = uv::wait(account->getRedelegations());
    EXPECT_GE(redelegations.size(), 1) << fmt::format(
        "Expecting at least 1 redelegation here for Obelix (explorer link : {}).",
        mintscanExplorer);
}

// FIXME This test fails ; put at the end because it also messes up the other tests
// This test is also highly dependent on external state ( how well the chain is
// doing). Until a better solution is found, this test is deactivated
// TEST_F(CosmosLikeWalletSynchronization, SuccessiveSynchronizations) {
//     std::string hexPubKey(ledger::testing::cosmos::DEFAULT_HEX_PUB_KEY);
//
//     std::shared_ptr<CosmosLikeAccount> account;
//     std::shared_ptr<AbstractWallet> wallet;
//
//     setupTest(account, wallet, hexPubKey);
//
//     // First synchro
//     performSynchro(account);
//     auto blockHeight1 = wait(account->getLastBlock()).height;
//
//     // Wait 30s (new Cosmos block every 7s)
//     fmt::print("Waiting new Cosmos block for 30s...\n");
//     std::flush(std::cerr);
//     std::flush(std::cout);
//     std::this_thread::sleep_for(std::chrono::seconds(30));
//
//     // Second synchro
//     // FIXME Fails due to limitation of test framework??
//     performSynchro(account);
//     auto blockHeight2 = wait(account->getLastBlock()).height;
//
//     EXPECT_NE(blockHeight1, blockHeight2);
// }

