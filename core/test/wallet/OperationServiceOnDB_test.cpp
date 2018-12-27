#include <api/ExecutionContext.hpp>
#include <gtest/gtest.h>
#include <CommonFixtureFunctions.hpp>
#include <Helpers.hpp>
#include <Mocks.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/common/InMemoryBlockchainDatabase.hpp>
#include <wallet/common/OperationServiceOnDB.hpp>

using namespace ledger::core;
using namespace ledger::core::tests;
using namespace testing;

class OperationServiceOnDBTest : public virtual Test , public CommonFixtureFunctions {
public: 
    typedef common::InMemoryBlockchainDatabase<BitcoinLikeNetwork::FilledBlock> InMemDB;
public:
    OperationServiceOnDBTest()
        : context(new SimpleExecutionContext())
        , fakeStableDB(context)
        , fakeUnstableDB(context)
        , fakePendingDB(context) {
        addressRegistryMock = std::make_shared<NiceMock<KeychainRegistryMock>>();
        stableDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
        unstableDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
        pendingDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
        operationService = std::make_shared<common::OperationServiceOnDB<BitcoinLikeNetwork::FilledBlock>>(
            context,
            addressRegistryMock,
            stableDBMock,
            unstableDBMock,
            pendingDBMock);
        linkMockDbToFake(stableDBMock, fakeStableDB);
        linkMockDbToFake(unstableDBMock, fakeUnstableDB);
        linkMockDbToFake(pendingDBMock, fakePendingDB);
    };

public:
    std::shared_ptr<SimpleExecutionContext> context;
    InMemDB fakeStableDB;
    InMemDB fakeUnstableDB;
    InMemDB fakePendingDB;
    std::shared_ptr<NiceMock<BlocksDBMock>> stableDBMock;
    std::shared_ptr<NiceMock<BlocksDBMock>> unstableDBMock;
    std::shared_ptr<NiceMock<BlocksDBMock>> pendingDBMock;
    std::shared_ptr<NiceMock<KeychainRegistryMock>> addressRegistryMock;
    std::shared_ptr<common::OperationServiceOnDB<BitcoinLikeNetwork::FilledBlock>> operationService;
};

TEST_F(OperationServiceOnDBTest, EmptyDatabases) {
    EXPECT_CALL(*stableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*unstableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*pendingDBMock, getLastBlock()).Times(1);
    auto pendingFuture = operationService->getPendingOperations();
    auto blockedFuture = operationService->getOperations(0, 1000000);
    context->wait();
    auto pendingResult = getFutureResult(pendingFuture);
    auto blockedResult = getFutureResult(blockedFuture);
    ASSERT_TRUE(pendingResult.empty());
    ASSERT_TRUE(blockedResult.empty());
}

TEST_F(OperationServiceOnDBTest, TransactionInStableOnly) {
    auto filledBlock = toFilledBlock(
        BL{ 10, "block 10",
        {
            TR{ { "X" },{ { "0", 10000 } } }
        } });
    fakeStableDB.addBlock(10, filledBlock);
    EXPECT_CALL(*stableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*unstableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*pendingDBMock, getLastBlock()).Times(1);
    auto pendingFuture = operationService->getPendingOperations();
    auto blockedFuture = operationService->getOperations(0, 1000000);
    context->wait();
    auto pendingResult = getFutureResult(pendingFuture);
    auto blockedResult = getFutureResult(blockedFuture);
    ASSERT_TRUE(pendingResult.empty());
    ASSERT_EQ(blockedResult.size(), 1);
    EXPECT_THAT(blockedResult[0].senders, Contains("X"));
    EXPECT_THAT(blockedResult[0].trust->getTrustLevel(), api::TrustLevel::TRUSTED);
}

TEST_F(OperationServiceOnDBTest, TransactionInUnStableOnly) {
    auto filledBlock = toFilledBlock(
        BL{ 10, "block 10",
        {
            TR{ { "X" },{ { "0", 10000 } } }
        } });
    fakeUnstableDB.addBlock(10, filledBlock);
    EXPECT_CALL(*stableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*unstableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*pendingDBMock, getLastBlock()).Times(1);
    auto pendingFuture = operationService->getPendingOperations();
    auto blockedFuture = operationService->getOperations(0, 1000000);
    context->wait();
    auto pendingResult = getFutureResult(pendingFuture);
    auto blockedResult = getFutureResult(blockedFuture);
    ASSERT_TRUE(pendingResult.empty());
    ASSERT_EQ(blockedResult.size(), 1);
    EXPECT_THAT(blockedResult[0].senders, Contains("X"));
    EXPECT_THAT(blockedResult[0].trust->getTrustLevel(), api::TrustLevel::UNTRUSTED);
}

TEST_F(OperationServiceOnDBTest, TransactionInPendingOnly) {
    auto filledBlock = toFilledBlock(
        BL{ 10, "block 10",
        {
            TR{ { "X" },{ { "0", 10000 } } }
        } });
    fakePendingDB.addBlock(0, filledBlock);
    EXPECT_CALL(*stableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*unstableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*pendingDBMock, getLastBlock()).Times(1);
    auto pendingFuture = operationService->getPendingOperations();
    auto blockedFuture = operationService->getOperations(0, 1000000);
    context->wait();
    auto pendingResult = getFutureResult(pendingFuture);
    auto blockedResult = getFutureResult(blockedFuture);
    ASSERT_TRUE(blockedResult.empty());
    ASSERT_EQ(pendingResult.size(), 1);
    ASSERT_THAT(pendingResult[0].senders, Contains("X"));
    EXPECT_THAT(pendingResult[0].trust->getTrustLevel(), api::TrustLevel::PENDING);
}

TEST_F(OperationServiceOnDBTest, TransactionInAllDB) {
    auto filledBlock = toFilledBlock(
        BL{ 10, "block 10",
        {
            TR{ { "Z" },{ { "0", 10000 } } }
        } });
    fakePendingDB.addBlock(0, filledBlock);
    fakeStableDB.addBlock(10, toFilledBlock(
        BL{ 10, "block 10",
        {
            TR{ { "X" },{ { "0", 10000 } } }
        } }));
    fakeUnstableDB.addBlock(10, toFilledBlock(
        BL{ 11, "block 11",
        {
            TR{ { "Y" },{ { "0", 10000 } } }
        } }));
    EXPECT_CALL(*stableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*unstableDBMock, getBlocks(0, 1000000)).Times(1);
    EXPECT_CALL(*pendingDBMock, getLastBlock()).Times(1);
    auto pendingFuture = operationService->getPendingOperations();
    auto blockedFuture = operationService->getOperations(0, 1000000);
    context->wait();
    auto pendingResult = getFutureResult(pendingFuture);
    auto blockedResult = getFutureResult(blockedFuture);
    ASSERT_EQ(blockedResult.size(), 2);
    EXPECT_THAT(pendingResult[0].senders, ElementsAre("Z"));
}
