#include "../integration/BaseFixture.h"
#include "Fixtures.hpp"

#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>

#include <wallet/pool/WalletPool.hpp>
#include <utils/DateUtils.hpp>

#include <gtest/gtest.h>

using namespace ledger::testing::cosmos;

class CosmosDBTest : public BaseFixture {
public:
 void SetUp() override
 {
     BaseFixture::SetUp();
#ifdef PG_SUPPORT
     const bool usePostgreSQL = true;
     configuration->putString(
         api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
     pool = newDefaultPool("postgres", "", configuration, usePostgreSQL);
#else
     pool = newDefaultPool();
#endif
     backend->enableQueryLogging(true);
 }

 void setupTest(
     std::shared_ptr<WalletPool> &pool,
     std::shared_ptr<CosmosLikeAccount> &account,
     std::shared_ptr<CosmosLikeWallet> &wallet)
 {
     auto configuration = DynamicObject::newInstance();
     configuration->putString(
         api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
         "44'/<coin_type>'/<account>'/<node>/<address>");
     wallet = std::dynamic_pointer_cast<CosmosLikeWallet>(
         wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "atom", configuration)));

     auto accountInfo = wait(wallet->getNextAccountCreationInfo());
     EXPECT_EQ(accountInfo.index, 0);
     accountInfo.publicKeys.push_back(hex::toByteArray(DEFAULT_HEX_PUB_KEY));

     account = createCosmosLikeAccount(wallet, accountInfo.index, accountInfo);
 }

 std::shared_ptr<WalletPool> pool;
};

TEST_F(CosmosDBTest, BasicDBTest) {
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(pool, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    const auto msg = setupSendMessage();
    auto tx = setupTransactionResponse(std::vector<Message>{ msg }, timeRef);

    // Test writing into DB
    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        CosmosLikeTransactionDatabaseHelper::putTransaction(sql, account->getAccountUid(), tx);
    }

    // Test reading from DB
    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        Transaction txRetrieved;
        auto result = CosmosLikeTransactionDatabaseHelper::getTransactionByHash(sql, tx.hash, txRetrieved);
        EXPECT_EQ(result, true);

        assertSameTransaction(tx, txRetrieved);

        // TODO Test other (all?) message types
        auto sendMsg = boost::get<MsgSend>(tx.messages[0].content);
        auto sendMsgRetrieved = boost::get<MsgSend>(txRetrieved.messages[0].content);
        EXPECT_EQ(sendMsgRetrieved.fromAddress, sendMsg.fromAddress);
        EXPECT_EQ(sendMsgRetrieved.toAddress, sendMsg.toAddress);
        EXPECT_EQ(sendMsgRetrieved.amount.size(), 1);
        EXPECT_EQ(sendMsgRetrieved.amount[0].amount, sendMsg.amount[0].amount);
        EXPECT_EQ(sendMsgRetrieved.amount[0].denom, sendMsg.amount[0].denom);
    }

}

TEST_F(CosmosDBTest, OperationQueryTest) {
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(pool, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    const auto msg = setupSendMessage();
    const auto tx = setupTransactionResponse(std::vector<Message>{ msg }, timeRef);

    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.set_log_stream(&std::cerr);
        account->putTransaction(sql, tx);
    }

    {
        auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
        ASSERT_EQ(ops.size(), 1);
        auto op = ops[0];

        /* TODO
        EXPECT_EQ(op->getAccountIndex(), account->getIndex());
        EXPECT_EQ(op->getAmount(), ?);
        EXPECT_EQ(op->getBlockHeight(), ?);
        EXPECT_EQ(op->getCurrency(), ?);
        EXPECT_EQ(op->getDate(), ?);
        EXPECT_EQ(op->getFees(), ?);
        EXPECT_EQ(op->getOperationType(), ?);
        EXPECT_EQ(op->getPreferences(), ?);
        EXPECT_EQ(op->getRecipients(), ?);
        EXPECT_EQ(op->getSenders(), ?);
        EXPECT_EQ(op->getTrust(), ?);
        EXPECT_EQ(op->getUid(), ?);
        EXPECT_EQ(op->isComplete(), ?);
        */

        // auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
        auto cosmosOp = op->asCosmosLikeOperation();

        auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();
        assertSameTransaction(tx, txRetrieved);

        assertSameSendMessage(tx.messages[0], txRetrieved.messages[0]);
    }
}

TEST_F(CosmosDBTest, UnsuportedMsgTypeTest) {
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(pool, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    const auto msg = setupSendMessage();
    auto tx = setupTransactionResponse(std::vector<Message>{ msg }, timeRef);

    // Change message type
    tx.messages[0].type = "unknown-message-type";

    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.set_log_stream(&std::cerr);
        account->putTransaction(sql, tx);
    }

    {
        auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
        ASSERT_EQ(ops.size(), 1);

        auto op = ops[0];
        // auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
        auto cosmosOp = op->asCosmosLikeOperation();
        const auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();

        assertSameTransaction(tx, txRetrieved);
    }
}

TEST_F(CosmosDBTest, MultipleMsgTest) {
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(pool, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    const auto msgSend = setupSendMessage();
    const auto msgVote = setupVoteMessage();
    const auto tx = setupTransactionResponse(std::vector<Message>{ msgSend, msgVote }, timeRef);

    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.set_log_stream(&std::cerr);
        account->putTransaction(sql, tx);
    }

    {
        auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
        ASSERT_EQ(ops.size(), 2);

        {
            auto op = ops[0];
            // auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
            auto cosmosOp = op->asCosmosLikeOperation();
            ASSERT_NE(cosmosOp, nullptr);
            ASSERT_NE(cosmosOp->getMessage(), nullptr);
            ASSERT_NE(cosmosOp->getTransaction(), nullptr);
            auto txPtr = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction());
            ASSERT_NE(txPtr, nullptr);
            auto txRetrieved = txPtr->getRawData();
            auto msgPtr = std::dynamic_pointer_cast<CosmosLikeMessage>(cosmosOp->getMessage());
            ASSERT_NE(msgPtr, nullptr);
            auto msgRetrieved = msgPtr->getRawData();

            assertSameTransaction(tx, txRetrieved);
            assertSameSendMessage(msgSend, msgRetrieved);
        }

        {
            auto op = ops[1];
            // auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
            // auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();
            // auto msgRetrieved = std::dynamic_pointer_cast<CosmosLikeMessage>(cosmosOp->getMessage())->getRawData();
            auto cosmosOp = op->asCosmosLikeOperation();
            ASSERT_NE(cosmosOp, nullptr);
            ASSERT_NE(cosmosOp->getMessage(), nullptr);
            ASSERT_NE(cosmosOp->getTransaction(), nullptr);
            auto txPtr = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction());
            ASSERT_NE(txPtr, nullptr);
            auto txRetrieved = txPtr->getRawData();
            auto msgPtr = std::dynamic_pointer_cast<CosmosLikeMessage>(cosmosOp->getMessage());
            ASSERT_NE(msgPtr, nullptr);
            auto msgRetrieved = msgPtr->getRawData();

            assertSameTransaction(tx, txRetrieved);
            assertSameVoteMessage(msgVote, msgRetrieved);
        }
    }
}
