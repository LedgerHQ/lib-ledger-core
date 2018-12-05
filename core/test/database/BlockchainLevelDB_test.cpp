#include <gtest/gtest.h>
#include <database/BlockchainLevelDB.hpp>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <TempDir.hpp>

using namespace ledger::core;

bool sameData(const std::vector<uint8_t>& l, const std::vector<uint8_t>& r) {
    if (l.size() != r.size())
        return false;
    return (std::memcmp(&l[0], &r[0], l.size()) == 0);
}

bool sameHeader(const db::DatabaseBlockHeader& l, const db::DatabaseBlockHeader& r) {
    return (l.height == r.height)
        && (l.hash == r.hash);
}

bool same(const db::DatabaseBlock& l, const db::DatabaseBlock& r) {
    return sameHeader(l.header, r.header) && sameData(l.data, r.data);
}

class BlockchainLevelDB : public ::testing::Test {
public:
    BlockchainLevelDB()
    : db(test::GetTempDirPath()) {
    }

    db::DatabaseBlock getBlock() {
        db::DatabaseBlock block;
        block.header.height = 100;
        block.header.hash = "MY BLOCK HASH";
        block.data = { 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
        return block;
    }

    static Option<db::DatabaseBlock> getDatablock(const Future<Option<db::DatabaseBlock>>& f) {
        EXPECT_TRUE(f.isCompleted());
        EXPECT_TRUE(f.getValue().hasValue());
        EXPECT_TRUE(f.getValue().getValue().isSuccess());
        return f.getValue().getValue().getValue();
    }

    static Option<db::DatabaseBlockHeader> getDatablockHeader(const Future<Option<db::DatabaseBlockHeader>>& f) {
        EXPECT_TRUE(f.isCompleted());
        EXPECT_TRUE(f.getValue().hasValue());
        EXPECT_TRUE(f.getValue().getValue().isSuccess());
        return f.getValue().getValue().getValue();
    }

public:
    db::BlockchainLevelDB db;
};

TEST_F(BlockchainLevelDB, SimpleGet) {
    auto block = getBlock();
    db.AddBlock(block);
    auto future = db.GetBlock(100);
    EXPECT_TRUE(future.isCompleted());
    auto tr = future.getValue();
    EXPECT_TRUE(tr.hasValue());
    auto opt = tr.getValue();
    db::DatabaseBlock readBlock = opt.getValue().getValue();
    EXPECT_TRUE(same(readBlock, block));
}

TEST_F(BlockchainLevelDB, ClearAll) {
    auto block = getBlock();
    db.AddBlock(block);
    auto future = db.GetBlock(100);
    EXPECT_TRUE(same(block, getDatablock(future).getValue()));
    db.CleanAll();
    future = db.GetBlock(100);
    EXPECT_TRUE(getDatablock(future).isEmpty());
}

TEST_F(BlockchainLevelDB, GetLastBlock) {
    auto block = getBlock();
    db.AddBlock(block);
    block.header.height = 101;
    db.AddBlock(block);
    block.header.height = 1000;
    db.AddBlock(block);
    block.header.height = 9;
    db.AddBlock(block);

    auto future = db.GetLastBlockHeader();
    EXPECT_EQ(1000, getDatablockHeader(future).getValue().height);
}

TEST_F(BlockchainLevelDB, GetLastBlockOnEmptyDB) {
    auto future = db.GetLastBlockHeader();
    EXPECT_TRUE(getDatablockHeader(future).isEmpty());
}
