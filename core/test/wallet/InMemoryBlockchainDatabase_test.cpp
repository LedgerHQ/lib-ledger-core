#include <gtest/gtest.h>
#include <chrono>
#include <wallet/common/InMemoryBlockchainDatabase.hpp>

#include <Helpers.hpp>

using namespace ledger::core;
using namespace ledger::core::tests;

typedef std::vector<BitcoinLikeNetwork::FilledBlock> VFB;

class InMemoryBlockchainDatabaseTest : public ::testing::Test {
public:
    typedef std::vector<BitcoinLikeNetwork::FilledBlock> Blocks;
    InMemoryBlockchainDatabaseTest()
        : simpleContext(new SimpleExecutionContext())
        , db(simpleContext){

    }
public:
    std::shared_ptr<SimpleExecutionContext> simpleContext;
    common::InMemoryBlockchainDatabase<BitcoinLikeNetwork> db;
};

static BitcoinLikeNetwork::FilledBlock FB(uint32_t height) {
    BitcoinLikeNetwork::FilledBlock filledBlock;
    filledBlock.header.height = height;
    filledBlock.header.hash = "block hash";
    filledBlock.header.createdAt = std::chrono::system_clock::time_point(std::chrono::hours(height));
    return filledBlock;
}

TEST_F(InMemoryBlockchainDatabaseTest, SimpleGet) {
    auto block = FB(100);
    db.addBlock(block);
    auto future = db.getBlock(100);
    EXPECT_TRUE(future.isCompleted());
    auto tr = future.getValue();
    EXPECT_TRUE(tr.hasValue());
    auto opt = tr.getValue();
    EXPECT_EQ(block, opt.getValue().getValue());
}

TEST_F(InMemoryBlockchainDatabaseTest, GetBlocks) {
    db.addBlock(FB(1));
    db.addBlock(FB(3));
    db.addBlock(FB(5));
    db.addBlock(FB(7));
    db.addBlock(FB(9));
    db.addBlock(FB(11));
    EXPECT_EQ(VFB{}, getFutureResult(db.getBlocks(0, 1)));
    EXPECT_EQ(VFB{ FB(1) }, getFutureResult(db.getBlocks(0, 2)));
    EXPECT_EQ(VFB{ FB(1) }, getFutureResult(db.getBlocks(0, 3)));
    EXPECT_EQ(VFB({ FB(1), FB(3) }), getFutureResult(db.getBlocks(0, 4)));
    EXPECT_EQ(VFB({ FB(1), FB(3) }), getFutureResult(db.getBlocks(1, 4)));
    EXPECT_EQ(VFB({ FB(3) }), getFutureResult(db.getBlocks(2, 4)));
    EXPECT_EQ(VFB{ FB(3) }, getFutureResult(db.getBlocks(3, 4)));
    EXPECT_EQ(VFB{}, getFutureResult(db.getBlocks(4, 4)));
    EXPECT_EQ(VFB({ FB(9), FB(11) }), getFutureResult(db.getBlocks(9, 20)));
    EXPECT_EQ(VFB{ FB(11) }, getFutureResult(db.getBlocks(11, 20)));
    EXPECT_EQ(VFB{}, getFutureResult(db.getBlocks(12, 20)));
}

TEST_F(InMemoryBlockchainDatabaseTest, GetLastBlock) {
    db.addBlock(FB(1));
    db.addBlock(FB(3));
    db.addBlock(FB(5));
    db.addBlock(FB(7));
    db.addBlock(FB(11));
    db.addBlock(FB(9));
    EXPECT_EQ(FB(11).header, getOptionFutureResult(db.getLastBlockHeader()));
    db.addBlock(FB(8));
    EXPECT_EQ(FB(11).header, getOptionFutureResult(db.getLastBlockHeader()));
    db.addBlock(FB(12));
    EXPECT_EQ(FB(12).header, getOptionFutureResult(db.getLastBlockHeader()));
}

TEST_F(InMemoryBlockchainDatabaseTest, CleanAll) {
    db.addBlock(FB(1));
    db.addBlock(FB(3));
    db.addBlock(FB(5));
    db.addBlock(FB(7));
    db.addBlock(FB(9));
    EXPECT_EQ(VFB({ FB(1), FB(3), FB(5), FB(7), FB(9)}), getFutureResult(db.getBlocks(0, 100)));
    db.CleanAll();
    EXPECT_EQ(VFB{}, getFutureResult(db.getBlocks(0, 100)));
}

TEST_F(InMemoryBlockchainDatabaseTest, RemoveBlocks) {
    db.addBlock(FB(1));
    db.addBlock(FB(3));
    db.addBlock(FB(5));
    db.addBlock(FB(7));
    db.addBlock(FB(9));
    db.addBlock(FB(11));
    EXPECT_EQ(VFB({ FB(1), FB(3), FB(5), FB(7), FB(9), FB(11) }), getFutureResult(db.getBlocks(0, 100)));
    db.removeBlocks(2, 3); // delete nothing
    EXPECT_EQ(VFB({ FB(1), FB(3), FB(5), FB(7), FB(9), FB(11) }), getFutureResult(db.getBlocks(0, 100)));
    db.removeBlocks(2, 5); // delete just 3
    EXPECT_EQ(VFB({ FB(1), FB(5), FB(7), FB(9), FB(11) }), getFutureResult(db.getBlocks(0, 100)));
    db.removeBlocks(2, 5); // delete nothing
    EXPECT_EQ(VFB({ FB(1), FB(5), FB(7), FB(9), FB(11) }), getFutureResult(db.getBlocks(0, 100)));
    db.removeBlocks(11, 100);
    EXPECT_EQ(VFB({ FB(1), FB(5), FB(7), FB(9)}), getFutureResult(db.getBlocks(0, 100)));
    db.removeBlocks(2, 8);
    EXPECT_EQ(VFB({ FB(1), FB(9) }), getFutureResult(db.getBlocks(0, 100)));
    db.removeBlocks(1, 10);
    EXPECT_EQ(VFB(), getFutureResult(db.getBlocks(0, 100)));
}