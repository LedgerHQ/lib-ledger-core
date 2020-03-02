#include "Fixtures.hpp"

#include <cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>
#include <cosmos/CosmosLikeOperationQuery.hpp>
#include <cosmos/CosmosLikeCurrencies.hpp>
#include <cosmos/CosmosLikeMessage.hpp>
#include <cosmos/CosmosLikeWallet.hpp>

#include <core/Services.hpp>
#include <core/utils/DateUtils.hpp>

#include <gtest/gtest.h>

using namespace ledger::testing::cosmos;

class CosmosDBTests : public BaseFixture {
public:
    void SetUp() override {
        BaseFixture::SetUp();
        //backend->enableQueryLogging(true);
    }

    void setupTest(std::shared_ptr<Services>& services,
                   std::shared_ptr<CosmosLikeAccount>& account,
                   std::shared_ptr<CosmosLikeWallet>& wallet) {

        services = newDefaultServices();

        auto walletStore = newWalletStore(services);
        wait(walletStore->addCurrency(currencies::ATOM));

        auto factory = std::make_shared<CosmosLikeWalletFactory>(currencies::ATOM, services);
        walletStore->registerFactory(currencies::ATOM, factory);

        auto configuration = DynamicObject::newInstance();
        //configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>");
        wallet = std::dynamic_pointer_cast<CosmosLikeWallet>(
                            wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "atom", configuration)));

        auto accountInfo = wait(wallet->getNextAccountCreationInfo());
        EXPECT_EQ(accountInfo.index, 0);
        accountInfo.publicKeys.push_back(hex::toByteArray(ledger::testing::cosmos::DEFAULT_HEX_PUB_KEY));

        account = ledger::testing::cosmos::createCosmosLikeAccount(wallet, accountInfo.index, accountInfo);
    }
};

TEST_F(CosmosDBTests, BasicDBTest) {

    std::shared_ptr<Services> services;
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(services, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    Message msg;
    setupSendMessage(msg, timeRef);

    Transaction tx;
    setupTransaction(tx, std::vector<Message>{ msg }, timeRef);

    // Test writing into DB
    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());
        CosmosLikeTransactionDatabaseHelper::putTransaction(sql, account->getAccountUid(), tx);
    }

    // Test reading from DB
    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());
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

TEST_F(CosmosDBTests, OperationQueryTest) {
    std::shared_ptr<Services> services;
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(services, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    Message msg;
    setupSendMessage(msg, timeRef);

    Transaction tx;
    setupTransaction(tx, std::vector<Message>{ msg }, timeRef);

    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());
        account->putTransaction(sql, tx);
    }

    {
        auto ops = wait(std::dynamic_pointer_cast<CosmosLikeOperationQuery>(account->queryOperations()->complete())->execute());
        EXPECT_EQ(ops.size(), 1);
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

        auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);

        auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();
        assertSameTransaction(tx, txRetrieved);

        assertSameSendMessage(tx.messages[0], txRetrieved.messages[0]);
    }
}

TEST_F(CosmosDBTests, UnsuportedMsgTypeTest) {
    std::shared_ptr<Services> services;
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(services, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    Message msg;
    setupSendMessage(msg, timeRef);

    Transaction tx;
    setupTransaction(tx, std::vector<Message>{ msg }, timeRef);

    // Change message type
    tx.messages[0].type = "unknown-message-type";

    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());
        account->putTransaction(sql, tx);
    }

    {
        auto ops = wait(std::dynamic_pointer_cast<CosmosLikeOperationQuery>(account->queryOperations()->complete())->execute());
        EXPECT_EQ(ops.size(), 1);

        auto op = ops[0];
        auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
        auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();

        assertSameTransaction(tx, txRetrieved);
    }
}

TEST_F(CosmosDBTests, MultipleMsgTest) {
    std::shared_ptr<Services> services;
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(services, account, wallet);

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    Message msgSend;
    setupSendMessage(msgSend, timeRef);

    Message msgVote;
    setupVoteMessage(msgVote, timeRef);

    Transaction tx;
    setupTransaction(tx, std::vector<Message>{ msgSend, msgVote }, timeRef);

    {
        soci::session sql(services->getDatabaseSessionPool()->getPool());
        account->putTransaction(sql, tx);
    }

    {
        auto ops = wait(std::dynamic_pointer_cast<CosmosLikeOperationQuery>(account->queryOperations()->complete())->execute());
        EXPECT_EQ(ops.size(), 2);

        {
            auto op = ops[0];
            auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
            auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();
            auto msgRetrieved = std::dynamic_pointer_cast<CosmosLikeMessage>(cosmosOp->getMessage())->getRawData();

            assertSameTransaction(tx, txRetrieved);
            assertSameSendMessage(msgSend, msgRetrieved);
        }

        {
            auto op = ops[1];
            auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
            auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();
            auto msgRetrieved = std::dynamic_pointer_cast<CosmosLikeMessage>(cosmosOp->getMessage())->getRawData();

            assertSameTransaction(tx, txRetrieved);
            assertSameVoteMessage(msgVote, msgRetrieved);
        }
    }
}