/*
 *
 * cosmos_synchronization.cpp
 * ledger-core
 *
 * Created by Gerry Agbobada on 2021-01-14
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include "../BaseFixture.h"
#include <test/cosmos/Fixtures.hpp>

#include <gtest/gtest.h>

#include <api/Configuration.hpp>
#include <api/BlockchainExplorerEngines.hpp>
#include <api/BlockchainObserverEngines.hpp>
#include <api/KeychainEngines.hpp>
#include <api/PoolConfiguration.hpp>
#include <collections/DynamicObject.hpp>
#include <math/BigInt.h>
#include <utils/DateUtils.hpp>
#include <utils/hex.h>

#include <api/CosmosConfigurationDefaults.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>
#include <wallet/cosmos/explorers/StargateGaiaCosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>

#include <wallet/cosmos/database/CosmosLikeOperationDatabaseHelper.hpp>

using namespace ledger::core;
using namespace ledger::testing::cosmos;

const std::string COSMOS_ENDPOINT = api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT;

class CosmosStargateWalletSynchronization : public BaseFixture {
public:
  void SetUp() override {
    BaseFixture::SetUp();
    auto worker = dispatcher->getSerialExecutionContext("worker");
    auto client =
        std::make_shared<HttpClient>(COSMOS_ENDPOINT, http, worker);

#ifdef PG_SUPPORT
    const bool usePostgreSQL = true;
    auto poolConfig = DynamicObject::newInstance();
    poolConfig->putString(api::PoolConfiguration::DATABASE_NAME,
                          "postgres://localhost:5432/test_db");
    pool = newDefaultPool("postgres", "", poolConfig, usePostgreSQL);
#else
    pool = newDefaultPool();
#endif

    explorer = std::make_shared<StargateGaiaCosmosLikeBlockchainExplorer>(
        worker, client, currencies::ATOM, std::make_shared<DynamicObject>());
  }

  void setupTest(std::shared_ptr<CosmosLikeAccount> &account,
                 std::shared_ptr<AbstractWallet> &wallet,
                 const std::string &pubKey) {

    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                             "44'/<coin_type>'/<account>'/<node>/<address>");
    configuration->putString(
        api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, COSMOS_ENDPOINT);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE,
                             api::BlockchainExplorerEngines::STARGATE_NODE);
    configuration->putString(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE,
                             api::BlockchainObserverEngines::STARGATE_NODE);
    wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61",
                                     "cosmos", configuration));

    auto accountInfo = wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(accountInfo.index, 0);
    accountInfo.publicKeys.push_back(hex::toByteArray(pubKey));

    account = ledger::testing::cosmos::createCosmosLikeAccount(
        wallet, accountInfo.index, accountInfo);
  }

  void performSynchro(const std::shared_ptr<CosmosLikeAccount> &account) {
    auto receiver =
        make_receiver([=](const std::shared_ptr<api::Event> &event) {
          fmt::print("Received event {}\n", api::to_string(event->getCode()));
          if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) {
            return;
          }
          ASSERT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

          auto balance = wait(account->getBalance());
          fmt::print("Balance: {} uatom\n", balance->toString());

          auto block = wait(account->getLastBlock());
          fmt::print("Block height: {}\n", block.height);
          EXPECT_GT(block.height, 0);

          dispatcher->stop();
        });

    account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),
                                      receiver);
    dispatcher->waitUntilStopped();
  }

  void TearDown() override {
    wait(pool->freshResetAll());
    BaseFixture::TearDown();
  }

  std::shared_ptr<WalletPool> pool;
  std::shared_ptr<StargateGaiaCosmosLikeBlockchainExplorer> explorer;
};

TEST_F(CosmosStargateWalletSynchronization, GetAccountWithExplorer) {

  auto account = ::wait(explorer->getAccount(STARGATE_DEFAULT_ADDRESS));
  EXPECT_EQ(account->address, STARGATE_DEFAULT_ADDRESS);
  EXPECT_EQ(account->accountNumber, "31094");
  EXPECT_EQ(account->withdrawAddress, STARGATE_DEFAULT_ADDRESS)
      << "Withdraw address has not been modified on this address";
}

TEST_F(CosmosStargateWalletSynchronization, InternalFeesMessageInTransaction) {
  /// This transaction contains 1 messages. One internal message to store fees
  /// is added after the transaction is retrieved from the network, hence the
  /// transaction should contain 2 messages, the last one being the one added
  /// specifically for the fees.
  const auto transaction = ::wait(explorer->getTransactionByHash(
      "88AF1F6010CE8C8BBBF4247B8AB723C469022F7329CAB906CB783C3377ECB005"));

  ASSERT_NE(transaction, nullptr);
  EXPECT_EQ(transaction->messages.size(), 2);
  EXPECT_EQ(transaction->logs.size(), 2);
  const auto feeMessageIndex = transaction->messages.size() - 1;
  const auto &feeMessage = transaction->messages[feeMessageIndex];
  const auto &feeMessageLog = transaction->logs[feeMessageIndex];

  auto reduceAmounts = [](const std::vector<cosmos::Coin> &coins) {
    return std::accumulate(std::begin(coins), std::end(coins), BigInt::ZERO,
                           [](BigInt s, const cosmos::Coin &coin) {
                             return s + BigInt::fromString(coin.amount);
                           });
  };

  // check the internal fees message content
  ASSERT_EQ(feeMessage.type, cosmos::constants::kMsgFees);
  const auto &feeMessageContent =
      boost::get<cosmos::MsgFees>(feeMessage.content);
  EXPECT_EQ(reduceAmounts(transaction->fee.amount),
            BigInt::fromString(feeMessageContent.fees.amount));
  EXPECT_EQ(feeMessageContent.payerAddress, STARGATE_DEFAULT_ADDRESS);

  // check the logs for the fees message
  EXPECT_EQ(feeMessageLog.messageIndex, feeMessageIndex);
  EXPECT_EQ(feeMessageLog.success, true);
  EXPECT_EQ(feeMessageLog.log, "");
}

TEST_F(CosmosStargateWalletSynchronization, GetErrorTransaction) {
  auto tx_hash =
      "F28FC2C8111B0E5DC90AA786DA61665CF4D6E615D6FE5B64FB9DFA7A5127BB9C";
  auto delegator = "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl";
  auto validator_src = "cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu";
  auto validator_dst = "cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk";
  auto raw_log = "out of gas in location: WritePerByte; gasWanted: 200000, gasUsed: 201836: out of gas";

  auto tx = ::wait(explorer->getTransactionByHash(tx_hash));
  ASSERT_EQ(tx->hash, tx_hash);
  EXPECT_EQ(tx->block->height, 502317);
  EXPECT_EQ(tx->logs.size(), 2) << "There should be a log for the redelegate and a log for the fees.";
  // Logs are stored twice now, but only in serialized types
  // DB will only ever store logs in cosmos_messages table anyway
  EXPECT_FALSE(tx->logs[0].success);
  EXPECT_EQ(tx->logs[0].log,
            "out of gas in location: WritePerByte; gasWanted: 200000, gasUsed: 201836: out of gas");
  EXPECT_FALSE(tx->messages[0].log.success);
  EXPECT_EQ(
      tx->messages[0].log.log,
      R"esc(out of gas in location: WritePerByte; gasWanted: 200000, gasUsed: 201836: out of gas)esc");
  EXPECT_EQ(tx->messages[0].type, cosmos::constants::kMsgBeginRedelegate);
  const cosmos::MsgBeginRedelegate &msg =
      boost::get<cosmos::MsgBeginRedelegate>(tx->messages[0].content);
  EXPECT_EQ(msg.delegatorAddress, delegator);
  EXPECT_EQ(msg.validatorSourceAddress, validator_src);
  EXPECT_EQ(msg.validatorDestinationAddress, validator_dst);
  EXPECT_EQ(msg.amount.amount, "100000");
  EXPECT_EQ(msg.amount.denom, "uatom");
  EXPECT_EQ(tx->fee.gas.toInt64(), 200000);
  EXPECT_EQ(tx->fee.amount[0].denom, "uatom");
  EXPECT_EQ(tx->fee.amount[0].amount, "0");
  EXPECT_EQ(tx->gasUsed, Option<std::string>("201836"));
}


TEST_F(CosmosStargateWalletSynchronization, GetSendWithExplorer) {
  auto tx_hash =
      "67400F958D93D17E940013916E58EFAA53CA6642C51F663D5ED6E6C84F715C04";

  auto receiver = "cosmos1s38sxqxdhc4r2vxk039zwjufeezvcr4w7r7rwf";
  // Note : the sender of the message is also the sender of the funds in this
  // transaction.
  auto sender = "cosmos1xxkueklal9vejv9unqu80w9vptyepfa95pd53u";

  auto tx = ::wait(explorer->getTransactionByHash(tx_hash));
  ASSERT_EQ(tx->hash, tx_hash);
  EXPECT_EQ(tx->block->height, 5369093);
  EXPECT_EQ(tx->logs.size(), 2) << "There should be the SEND message and the FEES message.";
  EXPECT_TRUE(tx->logs[0].success);
  EXPECT_TRUE(tx->messages[0].log.success);
  EXPECT_EQ(tx->messages[0].type, cosmos::constants::kMsgSend);
  const cosmos::MsgSend &msg =
      boost::get<cosmos::MsgSend>(tx->messages[0].content);
  EXPECT_EQ(msg.fromAddress, sender);
  EXPECT_EQ(msg.toAddress, receiver);
  EXPECT_EQ(msg.amount[0].amount, "55411729");
  EXPECT_EQ(msg.amount[0].denom, "uatom");
  EXPECT_EQ(tx->fee.gas.toInt64(), 105130);
  EXPECT_EQ(tx->fee.amount[0].denom, "uatom");
  EXPECT_EQ(tx->fee.amount[0].amount, "2629");
  EXPECT_EQ(tx->gasUsed, Option<std::string>("62369"));
}

TEST_F(CosmosStargateWalletSynchronization, GetDelegateWithExplorer) {
  auto delegator = "cosmos1tx4p5axt22vx20f8ssqz4ys6sk3zg9jvx5rf35";
  auto validator = "cosmosvaloper1sjllsnramtg3ewxqwwrwjxfgc4n4ef9u2lcnj0";

  auto filter = StargateGaiaCosmosLikeBlockchainExplorer::fuseFilters(
      {StargateGaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
           cosmos::constants::kEventTypeMessage,
           cosmos::constants::kAttributeKeyAction,
           cosmos::constants::kEventTypeDelegate),
       StargateGaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
           cosmos::constants::kEventTypeMessage,
           cosmos::constants::kAttributeKeySender, delegator)});
  auto bulk = ::wait(explorer->getTransactions(filter, 1, 10));
  auto transactions = bulk->transactions;
  ASSERT_TRUE(transactions.size() >= 1);
  bool foundTx = false;
  for (const auto &tx : transactions) {
    if (tx.hash ==
        "0BFBE51C87E46A5DCFED803315D323A55101EC98DB0EDDB369F770FF2765172A") {
      foundTx = true;
      EXPECT_EQ(tx.block->height, 5368645);
      EXPECT_EQ(tx.logs.size(), 2);
      EXPECT_TRUE(tx.logs[0].success);
      EXPECT_TRUE(tx.messages[0].log.success);
      EXPECT_EQ(tx.messages[0].type, cosmos::constants::kMsgDelegate);
      const cosmos::MsgDelegate &msg =
          boost::get<cosmos::MsgDelegate>(tx.messages[0].content);
      EXPECT_EQ(msg.delegatorAddress, delegator);
      EXPECT_EQ(msg.validatorAddress, validator);
      EXPECT_EQ(msg.amount.amount, "4000000");
      EXPECT_EQ(msg.amount.denom, "uatom");
      EXPECT_EQ(tx.fee.gas.toInt64(), 200000);
      EXPECT_EQ(tx.fee.amount[0].denom, "uatom");
      EXPECT_EQ(tx.fee.amount[0].amount, "5000");
      EXPECT_EQ(tx.gasUsed, Option<std::string>("147387"));
      break;
    }
  }
  ASSERT_TRUE(foundTx);
}

TEST_F(CosmosStargateWalletSynchronization, GetCurrentBlockWithExplorer) {
  auto block = ::wait(explorer->getCurrentBlock());
  EXPECT_TRUE(block->hash.size() > 0);
  EXPECT_TRUE(block->height > 0);
}

TEST_F(CosmosStargateWalletSynchronization, MediumXpubSynchronization) {
  auto walletName = "8d99cc44-9061-43a4-9edd-f908d2007926";
  auto pool = newDefaultPool();
  backend->enableQueryLogging(true);

  {
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                             "44'/<coin_type>'/<account>'/<node>/<address>");
    configuration->putString(
        api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, COSMOS_ENDPOINT);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE,
                             api::BlockchainExplorerEngines::STARGATE_NODE);
    configuration->putString(api::Configuration::BLOCKCHAIN_OBSERVER_ENGINE,
                             api::BlockchainObserverEngines::STARGATE_NODE);
    auto wallet = wait(
        pool->createWallet(walletName, currencies::ATOM.name, configuration));
    std::set<std::string> emittedOperations;
    {
      auto accountInfo = wait(wallet->getNextAccountCreationInfo());
      EXPECT_EQ(accountInfo.index, 0);

      accountInfo.publicKeys.push_back(
          hex::toByteArray(STARGATE_DEFAULT_HEX_PUB_KEY));
      auto account = ledger::testing::cosmos::createCosmosLikeAccount(
          wallet, accountInfo.index, accountInfo);

      auto receiver =
          make_receiver([&](const std::shared_ptr<api::Event> &event) {
            if (event->getCode() == api::EventCode::NEW_OPERATION) {
              auto uid = event->getPayload()
                             ->getString(api::Account::EV_NEW_OP_UID)
                             .value();
              EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
            }
          });
      auto address = wait(account->getFreshPublicAddresses())[0]->toString();
      EXPECT_EQ(address, STARGATE_DEFAULT_ADDRESS);
      std::cout << "Address : " << address << std::endl;
      pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(),
                                     receiver);

      receiver.reset();
      receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
          return;
        EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

        auto balance = wait(account->getBalance());
        fmt::print("Balance: {} uatom\n", balance->toString());
        auto txBuilder =
            std::dynamic_pointer_cast<CosmosLikeTransactionBuilder>(
                account->buildTransaction());
        dispatcher->stop();
      });

      auto restoreKey = account->getRestoreKey();
      account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),
                                        receiver);

      dispatcher->waitUntilStopped();

      auto block = wait(account->getLastBlock());
      fmt::print("Block height: {}\n", block.height);
      EXPECT_GT(block.height, 0);

      auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(
                          account->queryOperations()->complete())
                          ->execute());
      fmt::print("Ops: {}\n", ops.size());
      ASSERT_GT(ops.size(), 0);
      ASSERT_TRUE(ops[0]->getBlockHeight())
          << "The first operation should have a block height.";
      EXPECT_GT(ops[0]->getBlockHeight().value(), 0)
          << "The first operation should have a non 0 height.";
      EXPECT_LT(ops[0]->getBlockHeight().value(), block.height)
          << "The first operation should not have the same height as the last "
             "block.";

      const auto sequenceNumber = account->getInfo().sequence;
      const int sequence = std::atoi(sequenceNumber.c_str());
      EXPECT_GE(sequence, 0) << "Sequence was at 0 on 2020-11-27";

      const auto accountNumber = account->getInfo().accountNumber;
      EXPECT_EQ(accountNumber, "31094")
          << "Account number is a network constant for a given address";
    }
  }
}

TEST_F(CosmosStargateWalletSynchronization, Balances) {
  std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d4"
                          "23a5cc3c21d89a"; // Obelix

  std::shared_ptr<AbstractWallet> wallet;
  std::shared_ptr<CosmosLikeAccount> account;

  setupTest(account, wallet, hexPubKey);

  const std::string address = account->getKeychain()->getAddress()->toBech32();
  const std::string mintscanExplorer =
      fmt::format("https://mintscan.io/account/{}", address);

  const auto totalBalance = wait(account->getTotalBalance())->toLong();
  const auto delegatedBalance = wait(account->getDelegatedBalance())->toLong();
  const auto pendingRewards =
      wait(account->getPendingRewardsBalance())->toLong();
  const auto unbondingBalance = wait(account->getUnbondingBalance())->toLong();
  const auto spendableBalance = wait(account->getSpendableBalance())->toLong();

  EXPECT_LE(delegatedBalance, totalBalance)
      << "Delegated Coins fall under Total Balance, so delegatedBalance <= "
         "totalBalance";
  // Strict comparison here because
  // it's impossible to have pending rewards without
  // at least some delegatedBalance or unbonding balance
  EXPECT_LT(pendingRewards, totalBalance)
      << "Pending rewards fall under Total Balance, so pendingRewards < "
         "totalBalance";
  EXPECT_LE(unbondingBalance, totalBalance)
      << "Unbondings fall under Total Balance, so unbondingBalance <= "
         "totalBalance";
  EXPECT_LE(spendableBalance, totalBalance)
      << "Spendable Coins fall under Total Balance, so spendableBalance <= "
         "totalBalance";

  EXPECT_GE(pendingRewards, 1) << fmt::format(
      "Check {}  to see if the account really has <1 uatom of pending rewards",
      mintscanExplorer);
  EXPECT_GE(totalBalance, 1000000) << fmt::format(
      "Check {}  to see if the account really has <1 ATOM of total balance",
      mintscanExplorer);
  EXPECT_GE(delegatedBalance, 2800)
      << fmt::format("Check {}  to see if the account really has <0.002800 "
                     "ATOM of total delegations",
                     mintscanExplorer);
  EXPECT_GE(spendableBalance, 1000)
      << fmt::format("Check {}  to see if the account really has <0.001 ATOM "
                     "of available (spendable) balance",
                     mintscanExplorer);
  // Unbondings are moving too much to assert the amount.
}

TEST_F(CosmosStargateWalletSynchronization, AllTransactionsSynchronization) {
  // FIXME Use an account that has all expected types of transactions

  // cosmos184p7cfk3x455h0ym0rfj5jx0nprftdfdehschd One of Silicium accounts
  std::string hexPubKey = "03223f5343fcf72480a80688f372565469158c7c94a731b1367cc2ea93a9bcdf90";

  std::shared_ptr<CosmosLikeAccount> account;
  std::shared_ptr<AbstractWallet> wallet;

  setupTest(account, wallet, hexPubKey);

  performSynchro(account);

  auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(
                      account->queryOperations()->complete())
                      ->execute());
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

    std::cout << "Found operation type: "
              << cosmosOp->getMessage()->getRawMessageType() << std::endl;

    switch (cosmosOp->getMessage()->getMessageType()) {
    case api::CosmosLikeMsgType::MSGSEND:
      foundMsgSend = true;
      break;
    case api::CosmosLikeMsgType::MSGDELEGATE:
      foundMsgDelegate = true;
      break;
    case api::CosmosLikeMsgType::MSGBEGINREDELEGATE:
      foundMsgBeginRedelegate = true;
      break;
    case api::CosmosLikeMsgType::MSGUNDELEGATE:
      foundMsgUndelegate = true;
      break;
    case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL:
      foundMsgSubmitProposal = true;
      break;
    case api::CosmosLikeMsgType::MSGVOTE:
      foundMsgVote = true;
      break;
    case api::CosmosLikeMsgType::MSGDEPOSIT:
      foundMsgDeposit = true;
      break;
    case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD:
      foundMsgWithdrawDelegationReward = true;
      break;
    case api::CosmosLikeMsgType::MSGMULTISEND:
      foundMsgMultiSend = true;
      break;
    case api::CosmosLikeMsgType::MSGCREATEVALIDATOR:
      foundMsgCreateValidator = true;
      break;
    case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
      foundMsgEditValidator = true;
      break;
    case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS:
      foundMsgSetWithdrawAddress = true;
      break;
    case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD:
      foundMsgWithdrawDelegatorReward = true;
      break;
    case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION:
      foundMsgWithdrawValidatorCommission = true;
      break;
    case api::CosmosLikeMsgType::MSGUNJAIL:
      foundMsgUnjail = true;
      break;
    case api::CosmosLikeMsgType::UNSUPPORTED:
    case api::CosmosLikeMsgType::MSGFEES:
      break;
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

TEST_F(CosmosStargateWalletSynchronization, ValidatorSet) {
  auto set = ::wait(explorer->getActiveValidatorSet());

  EXPECT_EQ(set.size(), 125)
      << "currently cosmoshub-4 has 125 active validators";
}

TEST_F(CosmosStargateWalletSynchronization, ValidatorInfo) {
  const auto stargateAddress =
      "cosmosvaloper1sjllsnramtg3ewxqwwrwjxfgc4n4ef9u2lcnj0";
  const auto stargateValConsPubAddress =
      "cosmosvalconspub1zcjduepq6fpkt3qn9xd7u44478ypkhrvtx45uhfj3uhdny420hzgsssrvh3qnzwdpe";
  const auto mintscanAddress = fmt::format(
      "https://mintscan.io/validators/{}", stargateAddress);

  auto valInfo = ::wait(explorer->getValidatorInfo(stargateAddress));
  ASSERT_EQ(valInfo.operatorAddress, stargateAddress)
      << "We should fetch the expected validator";
  ASSERT_EQ(valInfo.consensusPubkey, stargateValConsPubAddress)
      << "We should fetch the expected validator";

  EXPECT_EQ(valInfo.validatorDetails.moniker, "🐠stake.fish");
  ASSERT_TRUE(valInfo.validatorDetails.identity);
  EXPECT_EQ(valInfo.validatorDetails.identity.value(), "90B597A673FC950E");
  ASSERT_TRUE(valInfo.validatorDetails.website);
  EXPECT_EQ(valInfo.validatorDetails.website.value(), "stake.fish");
  ASSERT_TRUE(valInfo.validatorDetails.details);
  EXPECT_EQ(valInfo.validatorDetails.details.value(),
            "We are the leading staking service provider for blockchain projects. Join our community to help secure networks and earn rewards. We know staking.");

  EXPECT_EQ(valInfo.commission.rates.maxRate, "1.000000000000000000");
  EXPECT_EQ(valInfo.commission.rates.maxChangeRate, "0.010000000000000000");
  EXPECT_GE(valInfo.commission.updateTime,
            DateUtils::fromJSON("2019-03-13T23:00:00Z"))
      << "As of this test writing, last update was on 2019-03-13T23:00:00Z. So "
         "updateTime should be at least as recent as this timestamp.";
  // Specify locale for std::stof -- Also, why we just pass strings instead of
  // parsing as much as possible
  setlocale(LC_NUMERIC, "C");
  EXPECT_LE(std::stof(valInfo.commission.rates.rate),
            std::stof(valInfo.commission.rates.maxRate));

  EXPECT_EQ(valInfo.unbondingHeight, 0)
      << fmt::format("Expecting Stake Fish to never have been jailed. Check "
                     "{} to see if the assertion holds",
                     mintscanAddress);
  EXPECT_FALSE(valInfo.unbondingTime)
      << fmt::format("Expecting Stake Fish to never have been jailed. Check "
                     "{} to see if the assertion holds",
                     mintscanAddress);
  EXPECT_EQ(valInfo.minSelfDelegation, "1")
      << fmt::format("Expecting Stake Fish to have '1' minimum self "
                     "delegation. Check {} to see if the assertion holds",
                     mintscanAddress);
  EXPECT_FALSE(valInfo.jailed)
      << fmt::format("Expecting Stake Fish to never have been jailed. Check "
                     "{} to see if the assertion holds",
                     mintscanAddress);
  EXPECT_GE(BigInt::fromString(valInfo.votingPower).toUint64(),
            BigInt::fromString("200000000").toUint64())
      << fmt::format("Expecting Stake Fish voting power to be > 400_000 ATOM. "
                     "Check {} to see if the assertion holds",
                     mintscanAddress);
  EXPECT_EQ(valInfo.activeStatus, "BOND_STATUS_BONDED") << fmt::format(
      "Expecting Stake Fish to be active (and that currently the explorer "
      "returns BOND_STATUS_BONDED for this status). Check {} to see if the assertion holds",
      mintscanAddress);

  EXPECT_EQ(valInfo.distInfo.validatorCommission, "n/a")
      << "Stargate explorer does not synchronize distInfo to save HTTP calls";
  EXPECT_EQ(valInfo.distInfo.selfBondRewards, "n/a")
      << "Stargate explorer does not synchronize distInfo to save HTTP calls";

  EXPECT_FALSE(valInfo.signInfo.tombstoned)
      << "Cosmostation is not expected to be tombstoned";
  EXPECT_GE(valInfo.signInfo.missedBlocksCounter, 0)
      << "This value cannot be negative";
  EXPECT_GE(valInfo.signInfo.jailedUntil,
            DateUtils::fromJSON("1970-01-01T00:00:00Z"))
      << "This value cannot be before epoch";
}

TEST_F(CosmosStargateWalletSynchronization, BalanceHistoryOperationQuery) {

  std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d4"
                          "23a5cc3c21d89a"; // Obelix

  std::shared_ptr<CosmosLikeAccount> account;
  std::shared_ptr<AbstractWallet> wallet;

  setupTest(account, wallet, hexPubKey);

  performSynchro(account);

  const auto &uid = account->getAccountUid();
  soci::session sql(wallet->getDatabase()->getPool());
  std::vector<Operation> operations;

  auto keychain = account->getKeychain();
  std::function<bool(const std::string &)> filter =
      [&keychain](const std::string addr) -> bool {
    return keychain->contains(addr);
  };

  // Get operations related to an account
  CosmosLikeOperationDatabaseHelper::queryOperations(sql, uid, operations,
                                                     filter);

  ASSERT_GE(operations.size(), 2)
      << "As of 2021-03-04, there are 2 operations picked up by the query";
}

TEST_F(CosmosStargateWalletSynchronization, GetAccountDelegations) {

  std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d4"
                          "23a5cc3c21d89a"; // Obelix

  std::shared_ptr<CosmosLikeAccount> account;
  std::shared_ptr<AbstractWallet> wallet;

  setupTest(account, wallet, hexPubKey);

  auto delegations = wait(account->getDelegations());
  EXPECT_GE(delegations.size(), 2);

  BigInt delegatedAmount;
  for (auto &delegation : delegations) {
    delegatedAmount =
        delegatedAmount + *(std::dynamic_pointer_cast<ledger::core::Amount>(
                                delegation->getDelegatedAmount())
                                ->value());
  }
  EXPECT_GE(delegatedAmount.toUint64(), 1000000UL); // 1 ATOM
}

TEST_F(CosmosStargateWalletSynchronization, GetAccountPendingRewards) {

  std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d4"
                          "23a5cc3c21d89a"; // Obelix

  std::shared_ptr<CosmosLikeAccount> account;
  std::shared_ptr<AbstractWallet> wallet;

  setupTest(account, wallet, hexPubKey);

  auto rewards = wait(account->getPendingRewards());
  EXPECT_GE(rewards.size(), 2);

  BigInt pendingReward;
  for (auto &reward : rewards) {
    pendingReward =
        pendingReward + *(std::dynamic_pointer_cast<ledger::core::Amount>(
                              reward->getRewardAmount())
                              ->value());
  }
  EXPECT_GE(pendingReward.toUint64(), 1000UL); // 1000 uATOM
}

namespace {

void GenericGasLimitEstimationTest(const std::string &strTx,
                                   CosmosStargateWalletSynchronization &t) {
  const auto tx = api::CosmosLikeTransactionBuilder::parseRawSignedTransaction(
      ledger::core::currencies::ATOM, strTx);
  const auto estimatedGasLimit = ::wait(t.explorer->getEstimatedGasLimit(tx, "1.0"));
  EXPECT_GT(estimatedGasLimit->toUint64(), 0)
      << "This test failing probably means that the payload in the gas "
         "estimation call needs to update the sequence number or the amount, "
         "to make the transaction valid again.";
}

} // namespace

TEST_F(CosmosStargateWalletSynchronization, GasLimitEstimationForTransfer) {
  const auto strTx =
      "{\"account_number\":\"31094\","
      "\"chain_id\":\"cosmoshub-4\","
      "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":"
      "\"200000\"},"
      "\"memo\":\"Sent from Ledger\","
      "\"msgs\":["
      "{\"type\":\"cosmos-sdk/MsgSend\","
      "\"value\":{"
      "\"amount\":{\"amount\":\"10\",\"denom\":\"uatom\"},"
      "\"from_address\":\"cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl\","
      "\"to_address\":\"cosmos16xyempempp92x9hyzz9wrgf94r6j9h5f06pxxv\""
      "}}],"
      "\"sequence\":\"63\"}";
  GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosStargateWalletSynchronization,
       GasLimitEstimationForWithdrawingRewards) {
  const auto strTx =
      "{\"account_number\":\"31094\","
      "\"chain_id\":\"cosmoshub-4\","
      "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":"
      "\"200020\"},"
      "\"memo\":\"Sent from Ledger\","
      "\"msgs\":["
      "{\"type\":\"cosmos-sdk/MsgWithdrawDelegationReward\""
      ",\"value\":{"
      "\"delegator_address\":\"cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl\","
      "\"validator_address\":"
      "\"cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu\""
      "}}],"
      "\"sequence\":\"0\"}";
  GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosStargateWalletSynchronization, GasLimitEstimationForDelegation) {
  const auto strTx =
      "{\"account_number\":\"31094\","
      "\"chain_id\":\"cosmoshub-4\","
      "\"fee\":{\"amount\":[{\"amount\":\"500\",\"denom\":\"uatom\"}],\"gas\":"
      "\"200000\"},"
      "\"memo\":\"Sent from Ledger\","
      "\"msgs\":["
      "{\"type\":\"cosmos-sdk/MsgDelegate\","
      "\"value\":{"
      "\"amount\":{\"amount\":\"10\",\"denom\":\"uatom\"},"
      "\"delegator_address\":\"cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl\","
      "\"validator_address\":"
      "\"cosmosvaloper1sjllsnramtg3ewxqwwrwjxfgc4n4ef9u2lcnj0\""
      "}}],"
      "\"sequence\":\"63\"}";
  GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosStargateWalletSynchronization, GasLimitEstimationForUnDelegation) {
  const auto strTx =
      "{\"account_number\":\"31094\","
      "\"chain_id\":\"cosmoshub-4\","
      "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":"
      "\"200000\"},"
      "\"memo\":\"Sent from Ledger\","
      "\"msgs\":["
      "{\"type\":\"cosmos-sdk/MsgUndelegate\","
      "\"value\":{"
      "\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
      "\"delegator_address\":\"cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl\","
      "\"validator_address\":"
      "\"cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu\""
      "}}],"
      "\"sequence\":\"0\"}";
  GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosStargateWalletSynchronization, GasLimitEstimationForRedelegation) {
  const auto strTx =
      "{\"account_number\":\"31094\","
      "\"chain_id\":\"cosmoshub-4\","
      "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":"
      "\"200000\"},"
      "\"memo\":\"Sent from Ledger\","
      "\"msgs\":["
      "{\"type\":\"cosmos-sdk/MsgBeginRedelegate\","
      "\"value\":{\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
      "\"delegator_address\":\"cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl\","
      "\"validator_dst_address\":"
      "\"cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu\","
      "\"validator_src_address\":"
      "\"cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk\""
      "}}],"
      "\"sequence\":\"0\"}";
  GenericGasLimitEstimationTest(strTx, *this);
}

TEST_F(CosmosStargateWalletSynchronization, PendingUnbondings) {
  std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d4"
                          "23a5cc3c21d89a"; // Obelix

  std::shared_ptr<CosmosLikeAccount> account;
  std::shared_ptr<AbstractWallet> wallet;

  setupTest(account, wallet, hexPubKey);
  const std::string address = account->getKeychain()->getAddress()->toBech32();
  const std::string mintscanExplorer =
      fmt::format("https://mintscan.io/account/{}", address);

  // First synchro
  performSynchro(account);

  auto unbondings = wait(account->getUnbondings());
  EXPECT_GE(unbondings.size(), 1) << fmt::format(
      "Expecting at least 1 unbonding here for Obelix (explorer link : {}).",
      mintscanExplorer);
}

TEST_F(CosmosStargateWalletSynchronization, PendingRedelegations) {
  std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d4"
                          "23a5cc3c21d89a"; // Obelix

  std::shared_ptr<CosmosLikeAccount> account;
  std::shared_ptr<AbstractWallet> wallet;

  setupTest(account, wallet, hexPubKey);
  const std::string address = account->getKeychain()->getAddress()->toBech32();
  const std::string mintscanExplorer =
      fmt::format("https://mintscan.io/account/{}", address);

  // First synchro
  performSynchro(account);

  auto redelegations = wait(account->getRedelegations());
  EXPECT_GE(redelegations.size(), 1) << fmt::format(
      "Expecting at least 1 redelegation here for Obelix (explorer link : {}).",
      mintscanExplorer);
}

// FIXME This test fails ; put at the end because it also messes up the other
// tests This test is also highly dependent on external state ( how well the
// chain is doing). Until a better solution is found, this test is deactivated
// TEST_F(CosmosStargateWalletSynchronization, SuccessiveSynchronizations) {
//     std::string
//     hexPubKey(ledger::testing::cosmos::STARGATE_DEFAULT_HEX_PUB_KEY);
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
