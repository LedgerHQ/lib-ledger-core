#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <iostream>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/common/BlocksSynchronizer.hpp>

using namespace ledger::core;

class ExplorerMock : public ExplorerV2<BitcoinLikeNetwork> {
public:
    MOCK_METHOD0(startSession, Future<void *>());
    MOCK_METHOD1(killSession, Future<Unit>(void *session));
    MOCK_METHOD3(getTransactions, FuturePtr<TransactionBulk>(const std::vector<std::string>& addresses, Option<std::string> fromBlockHash, Option<void*> session));
    MOCK_METHOD0(getCurrentBlock, FuturePtr<Block>());
    MOCK_METHOD1(getRawTransaction, Future<Bytes> (const std::string& transactionHash));
    MOCK_METHOD1(getTransactionByHash, FuturePtr<Transaction>(const std::string& transactionHash));
    MOCK_METHOD1(pushTransaction, Future<std::string> (const std::vector<uint8_t>& transaction));
    MOCK_METHOD0(getTimestamp, Future<int64_t>());
};

class KeychainMock : public Keychain<BitcoinLikeNetwork> {
public:

};

class BlocksDBMock : public BlockchainDatabase<BitcoinLikeNetwork> {
public:
    MOCK_METHOD1(addBlock, Future<Unit>(FilledBlock& blocks));

    MOCK_METHOD1(addBlocks, Future<Unit> (const std::vector<FilledBlock>& blocks));
    
    MOCK_METHOD2(removeBlocks, Future<Unit>(int heightFrom, int heightTo));
    
    MOCK_METHOD1(removeBlocksUpTo, Future<Unit>(int heightTo));
};

class BlockSyncTest : public ::testing::Test {
public:
    BlockSyncTest() {
        firstBlock = std::make_shared<BitcoinLikeNetwork::Block>();
        lastBlock = std::make_shared<BitcoinLikeNetwork::Block>();
        _explorerMock = std::make_shared<ExplorerMock>();
        _keychainMock = std::make_shared<KeychainMock>();
        _blocksDBMock = std::make_shared<BlocksDBMock>();
        synchronizer = std::make_shared<common::BlocksSynchronizer<BitcoinLikeNetwork>>(
            ImmediateExecutionContext::INSTANCE,
            _explorerMock,
            _keychainMock,
            _blocksDBMock,
            1,
            1);
    }
public:
    std::shared_ptr<common::BlocksSynchronizer<BitcoinLikeNetwork>> synchronizer;
    std::shared_ptr<BitcoinLikeNetwork::Block> firstBlock;
    std::shared_ptr<BitcoinLikeNetwork::Block> lastBlock;
    std::shared_ptr<ExplorerMock> _explorerMock;
    std::shared_ptr<KeychainMock> _keychainMock;
    std::shared_ptr<BlocksDBMock> _blocksDBMock;
};

TEST_F(BlockSyncTest, Insert) {
    synchronizer->synchronize(firstBlock, lastBlock);
}
