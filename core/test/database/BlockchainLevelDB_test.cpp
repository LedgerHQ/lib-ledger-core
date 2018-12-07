#include <gtest/gtest.h>
#include <database/BlockchainLevelDB.hpp>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <TempDir.hpp>

using namespace ledger::core;

class BlockchainLevelDB : public ::testing::Test {
public:
    BlockchainLevelDB()
    : db(test::GetTempDirPath()) {
    }
    
    template<typename T>
    static T getFutureResult(const Future<T>& f) {
        EXPECT_TRUE(f.isCompleted());
        EXPECT_TRUE(f.getValue().hasValue());
        EXPECT_TRUE(f.getValue().getValue().isSuccess());
        return f.getValue().getValue().getValue();
    }

public:
    db::BlockchainLevelDB db;
};

TEST_F(BlockchainLevelDB, SimpleGet) {
    db::BlockchainDB::RawBlock block{ 0x00, 0x01, 0x02, 0x03 };
    db.AddBlock(100, block);
    auto future = db.GetBlock(100);
    EXPECT_TRUE(future.isCompleted());
    auto tr = future.getValue();
    EXPECT_TRUE(tr.hasValue());
    auto opt = tr.getValue();
    EXPECT_EQ(block, opt.getValue().getValue());
}

TEST_F(BlockchainLevelDB, ClearAll) {
    db::BlockchainDB::RawBlock block{ 0x00, 0x01, 0x02, 0x03 };
    db.AddBlock(100, block);
    auto future = db.GetBlock(100);
    EXPECT_EQ(block, getFutureResult(future).getValue());
    db.CleanAll();
    future = db.GetBlock(100);
    EXPECT_TRUE(getFutureResult(future).isEmpty());
}

TEST_F(BlockchainLevelDB, GetLastBlock) {
    db::BlockchainDB::RawBlock block{ 0x00, 0x01, 0x02, 0x03 };
    db::BlockchainDB::RawBlock lastblock{ 0x05, 0x04, 0x03, 0x02 };
    db.AddBlock(200, block);
    db.AddBlock(201, block);
    db.AddBlock(1000, lastblock);
    db.AddBlock(100, block);
    db.AddBlock(9, block);

    auto future = db.GetLastBlock();
    EXPECT_EQ(lastblock, getFutureResult(future).getValue());
}

TEST_F(BlockchainLevelDB, GetLastBlockOnEmptyDB) {
    auto future = db.GetLastBlock();
    EXPECT_TRUE(getFutureResult(future).isEmpty());
}
