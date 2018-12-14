#include <gtest/gtest.h>
#include <wallet/common/InMemoryBlockchainDatabase.hpp>

#include <Helpers.hpp>

using namespace ledger::core;

template<typename T>
static T getFutureResult(const Future<T>& f) {
    EXPECT_TRUE(f.isCompleted());
    EXPECT_TRUE(f.getValue().hasValue());
    EXPECT_TRUE(f.getValue().getValue().isSuccess());
    return f.getValue().getValue().getValue();
}

template<typename T>
static T getOptionFutureResult(const Future<Option<T>>& f) {
    Option<T> o = getFutureResult(f);
    EXPECT_TRUE(o.hasValue());
    return o.getValue();
}

template<typename T>
static bool checkOptionFutureHasNoValue(const Future<Option<T>>& f) {
    Option<T> o = getFutureResult(f);
    EXPECT_TRUE(o.isEmpty());
}

class InMemoryBlockchainDatabaseTest : public ::testing::Test {
public:
    typedef std::vector<BitcoinLikeNetwork::FilledBlock> Blocks;
    InMemoryBlockchainDatabaseTest()
        : simpleContext(new tests::SimpleExecutionContext())
        , db(simpleContext){

    }
public:
    std::shared_ptr<tests::SimpleExecutionContext> simpleContext;
    common::InMemoryBlockchainDatabase<BitcoinLikeNetwork> db;
};

static BitcoinLikeNetwork::FilledBlock FB(uint32_t height) {
    BitcoinLikeNetwork::FilledBlock filledBlock;
    filledBlock.header.height = height;
    filledBlock.header.hash = "block hash";
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
    //EXPECT_EQ(block.header, opt.getValue().getValue().header);
}

/*
TEST(InMemoryBlockchainDatabaseTest, GetBlocks) {
    db::BlockchainDBInMemory db;
    db.AddBlock(1, { 0x01 });
    db.AddBlock(3, { 0x03 });
    db.AddBlock(5, { 0x05 });
    db.AddBlock(7, { 0x07 });
    db.AddBlock(9, { 0x09 });
    db.AddBlock(11, { 0x11 });
    EXPECT_EQ(Blocks(), getFutureResult(db.GetBlocks(0, 1)));
    EXPECT_EQ(Blocks({ {0x01} }), getFutureResult(db.GetBlocks(0, 2)));
    EXPECT_EQ(Blocks({ { 0x01 } }), getFutureResult(db.GetBlocks(0, 3)));
    EXPECT_EQ(Blocks({ { 0x01 }, { 0x03 } }), getFutureResult(db.GetBlocks(0, 4)));
    EXPECT_EQ(Blocks({ { 0x01 }, { 0x03 } }), getFutureResult(db.GetBlocks(1, 4)));
    EXPECT_EQ(Blocks({ { 0x03 } }), getFutureResult(db.GetBlocks(2, 4)));
    EXPECT_EQ(Blocks({ { 0x03 } }), getFutureResult(db.GetBlocks(3, 4)));
    EXPECT_EQ(Blocks(), getFutureResult(db.GetBlocks(4, 4)));
    EXPECT_EQ(Blocks({ { 0x09 }, { 0x11 } }), getFutureResult(db.GetBlocks(9, 20)));
    EXPECT_EQ(Blocks({ { 0x11 } }), getFutureResult(db.GetBlocks(11, 20)));
    EXPECT_EQ(Blocks(), getFutureResult(db.GetBlocks(12, 20)));
}

TEST(InMemoryBlockchainDatabaseTest, GetLastBlock) {
    db::BlockchainDBInMemory db;
    db.AddBlock(1, { 0x01 });
    db.AddBlock(3, { 0x03 });
    db.AddBlock(5, { 0x05 });
    db.AddBlock(7, { 0x07 });
    db.AddBlock(11, { 0x11 });
    db.AddBlock(9, { 0x09 });
    EXPECT_EQ(B{0x11}, getOptionFutureResult(db.GetLastBlock()));
    db.AddBlock(8, { 0x08 });
    EXPECT_EQ(B{ 0x11 }, getOptionFutureResult(db.GetLastBlock()));
    db.AddBlock(12, { 0x12 });
    EXPECT_EQ(B{ 0x12 }, getOptionFutureResult(db.GetLastBlock()));
}

TEST(InMemoryBlockchainDatabaseTest, CleanAll) {
    db::BlockchainDBInMemory db;
    db.AddBlock(1, { 0x01 });
    db.AddBlock(3, { 0x03 });
    db.AddBlock(5, { 0x05 });
    db.AddBlock(7, { 0x07 });
    db.AddBlock(9, { 0x09 });
    EXPECT_EQ(Blocks({ { 0x01 },{ 0x03 }, {0x05}, { 0x07 }, { 0x09 } }), getFutureResult(db.GetBlocks(0, 100)));
    db.CleanAll();
    EXPECT_EQ(Blocks(), getFutureResult(db.GetBlocks(0, 100)));
}

TEST(InMemoryBlockchainDatabaseTest, RemoveBlocks) {
    db::BlockchainDBInMemory db;
    db.AddBlock(1, { 0x01 });
    db.AddBlock(3, { 0x03 });
    db.AddBlock(5, { 0x05 });
    db.AddBlock(7, { 0x07 });
    db.AddBlock(9, { 0x09 });
    db.AddBlock(11, { 0x11 });
    EXPECT_EQ(Blocks({ { 0x01 },{ 0x03 },{ 0x05 },{ 0x07 },{ 0x09 }, {0x11} }), getFutureResult(db.GetBlocks(0, 100)));
    db.RemoveBlocks(2, 3); // delete nothing
    EXPECT_EQ(Blocks({ { 0x01 },{ 0x03 },{ 0x05 },{ 0x07 },{ 0x09 },{ 0x11 } }), getFutureResult(db.GetBlocks(0, 100)));
    db.RemoveBlocks(2, 5); // delete just 3
    EXPECT_EQ(Blocks({ { 0x01 },{ 0x05 },{ 0x07 },{ 0x09 },{ 0x11 } }), getFutureResult(db.GetBlocks(0, 100)));
    db.RemoveBlocks(2, 5); // delete nothing
    EXPECT_EQ(Blocks({ { 0x01 },{ 0x05 },{ 0x07 },{ 0x09 },{ 0x11 } }), getFutureResult(db.GetBlocks(0, 100)));
    db.RemoveBlocks(11, 100);
    EXPECT_EQ(Blocks({ { 0x01 },{ 0x05 },{ 0x07 },{ 0x09 }}), getFutureResult(db.GetBlocks(0, 100)));
    db.RemoveBlocks(2, 8);
    EXPECT_EQ(Blocks({ { 0x01 },{ 0x09 } }), getFutureResult(db.GetBlocks(0, 100)));
    db.RemoveBlocks(1, 10);
    EXPECT_EQ(Blocks(), getFutureResult(db.GetBlocks(0, 100)));
}

*/