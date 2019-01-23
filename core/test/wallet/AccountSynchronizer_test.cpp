#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <database/BlockchainDBInMemory.hpp>
#include <memory>
#include <wallet/NetworkTypes.hpp>
#include <wallet/common/AccountSynchronizer.hpp>
#include <events/ProgressNotifier.h>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>
#include <wallet/common/InMemoryBlockchainDatabase.hpp>
#include <wallet/StateManager.hpp>
#include <vector>
#include <spdlog/spdlog.h>

#include "CommonFixtureFunctions.hpp"
#include "Helpers.hpp"
#include "Mocks.hpp"

using namespace ledger;
using namespace ledger::core;
using namespace ledger::core::tests;
using namespace testing;

class AccountSyncTest : public virtual Test, public tests::CommonFixtureFunctions {
public:
    typedef common::InMemoryBlockchainDatabase<BitcoinLikeNetwork::FilledBlock> BlocksDatabase;
public:
    AccountSyncTest()
        : receivedFake(0, 0) // receive addresses are 0,1,2... change are 1000, 1001...
        , changeFake(0, 1000)
        , context(new SimpleExecutionContext())
        , fakeStableDB(context) {
        explorerMock = std::make_shared<NiceMock<ExplorerMock>>();
        keychainReceiveMock = std::make_shared<NiceMock<KeychainMock>>();
        keychainChangeMock = std::make_shared<NiceMock<KeychainMock>>();
        stableBlocksDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
        auto unstableBlocksDBMock = std::make_shared<NiceMock<BlocksDBMock>>();
        std::shared_ptr<api::ExecutionContext> xx = context;
        loggerSinkMock = std::make_shared<NiceMock<LoggerSinkMock>>();
        logger = std::make_shared<spdlog::logger>("unittestlogger", loggerSinkMock);
        BlockchainState<BitcoinLikeNetwork::FilledBlock> state(stableBlocksDBMock, unstableBlocksDBMock, std::vector<BitcoinLikeNetwork::Transaction>());
        stateManager = std::make_shared<StateManager<BitcoinLikeNetwork::FilledBlock>>(state);
    }

    void SetUp(uint32_t maxPossibleUnstableBlocks, const std::string& genesisBlockHash ) {
        common::SynchronizerConfiguration config(
            maxPossibleUnstableBlocks,
            1,
            1,
            200,
            genesisBlockHash);
                   
        synchronizer = std::make_shared<common::AccountSynchronizer<BitcoinLikeNetwork>>(
            stateManager,
            context,
            explorerMock,
            stableBlocksDBMock,
            keychainReceiveMock,
            keychainChangeMock,
            logger,
            config);
    }

    void setupFakeKeychains() {
        ON_CALL(*keychainReceiveMock, getNumberOfUsedAddresses()).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getNumberOfUsedAddresses));
        ON_CALL(*keychainReceiveMock, getAddresses(_, _)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getAddresses));
        ON_CALL(*keychainReceiveMock, markAsUsed(_)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::markAsUsed));
        ON_CALL(*keychainChangeMock, getNumberOfUsedAddresses()).WillByDefault(Invoke(&changeFake, &FakeKeyChain::getNumberOfUsedAddresses));
        ON_CALL(*keychainChangeMock, getAddresses(_, _)).WillByDefault(Invoke(&changeFake, &FakeKeyChain::getAddresses));
        ON_CALL(*keychainChangeMock, markAsUsed(_)).WillByDefault(Invoke(&changeFake, &FakeKeyChain::markAsUsed));
    };

    void setupFakeDatabases() {
        linkMockDbToFake(stableBlocksDBMock, fakeStableDB);
    }

    void setBlockchain(const std::vector<BL>& blockChain) {
        fakeExplorer.setBlockchain(toFilledBlocks(blockChain));
        ON_CALL(*explorerMock, getTransactions(_, _, _)).WillByDefault(Invoke(&fakeExplorer, &FakeExplorer::getTransactions));
        ON_CALL(*explorerMock, getCurrentBlock()).WillByDefault(Invoke(&fakeExplorer, &FakeExplorer::getCurrentBlock));
    }
    
    std::tuple <
        std::vector<BitcoinLikeNetwork::FilledBlock>,
        std::vector<BitcoinLikeNetwork::FilledBlock>,
        std::vector<BitcoinLikeNetwork::Transaction> > getBlocksFromState(const BlockchainState<BitcoinLikeNetwork::FilledBlock>& state) {
        auto stabelBlocksFuture = state.stableBlocks->getBlocks(0, 1000000);
        auto unstabelBlocksFuture = state.unstableBlocks->getBlocks(0, 1000000);
        context->wait();
        EXPECT_TRUE(stabelBlocksFuture.isCompleted());
        EXPECT_TRUE(stabelBlocksFuture.getValue().getValue().isSuccess());
        EXPECT_TRUE(unstabelBlocksFuture.isCompleted());
        EXPECT_TRUE(unstabelBlocksFuture.getValue().getValue().isSuccess());
        auto stableBlocks = getFutureResult(stabelBlocksFuture);
        auto unstableBlocks = getFutureResult(unstabelBlocksFuture);
        return std::make_tuple(stableBlocks, unstableBlocks, state.pendingTransactions);
    }
    
public:
    std::shared_ptr<StateManager<BitcoinLikeNetwork::FilledBlock>> stateManager;
    std::shared_ptr<SimpleExecutionContext> context;
    BlocksDatabase fakeStableDB;
    std::shared_ptr<core::AccountSynchronizer<BitcoinLikeNetwork>> synchronizer;
    std::shared_ptr<NiceMock<ExplorerMock>> explorerMock;
    std::shared_ptr<NiceMock<KeychainMock>> keychainReceiveMock;
    std::shared_ptr<NiceMock<KeychainMock>> keychainChangeMock;
    std::shared_ptr<NiceMock<BlocksDBMock>> stableBlocksDBMock;
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<NiceMock<LoggerSinkMock>> loggerSinkMock;
    FakeKeyChain receivedFake;
    FakeKeyChain changeFake;
    FakeExplorer fakeExplorer;
};

TEST_F(AccountSyncTest, NoStableBlocksInBlockchain) {
    // In this test we check that when we have number of blocks that is smaller then numberOfUnstable blocks
    // only unstable DB is updated
    SetUp(1, "block 1");
    setupFakeKeychains();
    setupFakeDatabases();
    std::vector<BL> bch =
    {
        BL{ 1, "block 1",
        {
            TR{ { { "X", 0 } }, { { "0", 10000 } } }
        } }
    };
    setBlockchain(bch);
    EXPECT_CALL(*stableBlocksDBMock, addBlock(_, _)).Times(0); // no stable blocks
    auto f = synchronizer->synchronize()->getFuture();
    context->wait();
    ASSERT_TRUE(f.isCompleted());
    EXPECT_TRUE(f.getValue().getValue().isSuccess());
    std::vector<BitcoinLikeNetwork::FilledBlock> stableBlocks;
    std::vector<BitcoinLikeNetwork::FilledBlock> unstableBlocks;
    std::vector<BitcoinLikeNetwork::Transaction> transactions;
    std::tie(stableBlocks, unstableBlocks, transactions) = getBlocksFromState(stateManager->getState());
    EXPECT_TRUE(stableBlocks.empty());
    ASSERT_EQ(unstableBlocks.size(), 1);
    EXPECT_THAT(unstableBlocks[0], Truly(Same(bch[0])));
}

TEST_F(AccountSyncTest, HappyPath) {
    // In this test we check that few blocks where added to to stable blocks DB
    // as well as to the unstable
    SetUp(3, "block 1");
    setupFakeKeychains();
    setupFakeDatabases();
    std::vector<BL> bch;
    for (uint32_t i = 1; i <= 5; ++i)
        bch.push_back(BL{ i, "block " + boost::lexical_cast<std::string>(i),{ TR{ { { "X", 0 } }, { { "0", 10000 } } } } });
    setBlockchain(bch);
    {
        testing::Sequence s;
        // two blocks goes to stable
        EXPECT_CALL(*stableBlocksDBMock, addBlock(bch[0].height, Truly(Same(bch[0])))).Times(1);
        EXPECT_CALL(*stableBlocksDBMock, addBlock(bch[1].height, Truly(Same(bch[1])))).Times(1);
        // and three goes to unstable
    }
    auto f = synchronizer->synchronize()->getFuture();
    context->wait();
    ASSERT_TRUE(f.isCompleted());
    EXPECT_TRUE(f.getValue().getValue().isSuccess());

    std::vector<BitcoinLikeNetwork::FilledBlock> stableBlocks;
    std::vector<BitcoinLikeNetwork::FilledBlock> unstableBlocks;
    std::vector<BitcoinLikeNetwork::Transaction> transactions;
    std::tie(stableBlocks, unstableBlocks, transactions) = getBlocksFromState(stateManager->getState());
    EXPECT_EQ(stableBlocks.size(), 2);
    EXPECT_THAT(stableBlocks[0], Truly(Same(bch[0])));
    EXPECT_THAT(stableBlocks[1], Truly(Same(bch[1])));
    ASSERT_EQ(unstableBlocks.size(), 3);

    EXPECT_THAT(unstableBlocks[0], Truly(Same(bch[2])));
    EXPECT_THAT(unstableBlocks[1], Truly(Same(bch[3])));
    EXPECT_THAT(unstableBlocks[2], Truly(Same(bch[4])));
}

TEST_F(AccountSyncTest, NoUpdatesNeeded) {
    // In this test we check that no new blocks where added to the stable db
    // and unstable db was created with few blocks
    SetUp(3, "block 1");
    setupFakeKeychains();
    setupFakeDatabases();
    fakeStableDB.addBlock(2, toFilledBlock(BL{ 2, "block 2", {} }));
    std::vector<BL> bch;
    for (uint32_t i = 1; i <= 5; ++i)
        bch.push_back(BL{ i, "block " + boost::lexical_cast<std::string>(i),{ TR{ { { "X", 0 } }, { { "0", 10000 } } } } });
    EXPECT_CALL(*stableBlocksDBMock, addBlock(_, _)).Times(1);
    setBlockchain(bch);
    auto f = synchronizer->synchronize()->getFuture();
    context->wait();
    ASSERT_TRUE(f.isCompleted());
    EXPECT_TRUE(f.getValue().getValue().isSuccess());

    std::vector<BitcoinLikeNetwork::FilledBlock> stableBlocks;
    std::vector<BitcoinLikeNetwork::FilledBlock> unstableBlocks;
    std::vector<BitcoinLikeNetwork::Transaction> transactions;
    std::tie(stableBlocks, unstableBlocks, transactions) = getBlocksFromState(stateManager->getState());
    EXPECT_EQ(stableBlocks.size(), 1);
    ASSERT_EQ(unstableBlocks.size(), 3);
    
    EXPECT_THAT(unstableBlocks[0], Truly(Same(bch[2])));
    EXPECT_THAT(unstableBlocks[1], Truly(Same(bch[3])));
    EXPECT_THAT(unstableBlocks[2], Truly(Same(bch[4])));
}
