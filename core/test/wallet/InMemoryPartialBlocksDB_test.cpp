#include <gtest/gtest.h>
#include <iostream>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>
#include <wallet/NetworkTypes.hpp>

using namespace ledger::core;

TEST(InMemoryPartialBlocksDB, Insert) {
    const uint32_t blockHeight = 11;
    common::InMemoryPartialBlocksDB<BitcoinLikeNetwork::Transaction> db;
    BitcoinLikeNetwork::Transaction tr;
    BitcoinLikeNetwork::Block block;
    block.hash = "0x0001";
    block.height = blockHeight;
    tr.block = BitcoinLikeNetwork::Block();
    tr.block = block;
    tr.hash = "transaction hash";
    db.addTransaction(tr);
    db.addTransaction(tr);
    auto res = db.getTransactions(blockHeight);
    EXPECT_EQ(res.size(), 1);
    EXPECT_EQ(res[0].hash, "transaction hash");
    tr.hash = "another hash";
    db.addTransaction(tr);
    EXPECT_EQ(db.getTransactions(blockHeight).size(), 2);
}

TEST(InMemoryPartialBlocksDB, Delete) {
    const uint32_t blockHeight = 11;
    common::InMemoryPartialBlocksDB<BitcoinLikeNetwork::Transaction> db;
    BitcoinLikeNetwork::Transaction tr;
    BitcoinLikeNetwork::Block block;
    block.hash = "0x0001";
    block.height = blockHeight;
    tr.block = BitcoinLikeNetwork::Block();
    tr.block = block;
    tr.hash = "transaction hash";
    db.addTransaction(tr);
    auto res = db.getTransactions(blockHeight);
    EXPECT_EQ(res.size(), 1);
    EXPECT_EQ(res[0].hash, "transaction hash");
    db.removeBlock(blockHeight);
    EXPECT_EQ(db.getTransactions(blockHeight).size(), 0);
}