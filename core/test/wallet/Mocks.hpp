#pragma once
#include <gmock/gmock.h>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <database/BlockchainDB.hpp>
#include <spdlog/sinks/sink.h>

namespace ledger {
    namespace core {
        namespace tests {
            class ExplorerMock : public ExplorerV2<BitcoinLikeNetwork> {
            public:
                MOCK_METHOD0(startSession, Future<void *>());
                MOCK_METHOD1(killSession, Future<Unit>(void *session));
                MOCK_METHOD3(getTransactions, Future<TransactionBulk>(const std::vector<std::string>& addresses, Option<std::string> fromBlockHash, Option<void*> session));
                MOCK_METHOD0(getCurrentBlock, Future<Block>());
                MOCK_METHOD1(getRawTransaction, Future<Bytes>(const std::string& transactionHash));
                MOCK_METHOD1(getTransactionByHash, Future<Transaction>(const std::string& transactionHash));
                MOCK_METHOD1(pushTransaction, Future<std::string>(const std::vector<uint8_t>& transaction));
                MOCK_METHOD0(getTimestamp, Future<int64_t>());
            };

            class KeychainMock : public Keychain {
            public:
                MOCK_METHOD0(getNumberOfUsedAddresses, uint32_t());
                MOCK_METHOD2(getAddresses, std::vector<std::string>(uint32_t startIndex, uint32_t count));
                MOCK_METHOD1(markAsUsed, void(const std::string& address));
            };

            class BlocksDBMock : public BlockchainDatabase<BitcoinLikeNetwork> {
            public:
                MOCK_METHOD1(addBlock, void(const BitcoinLikeNetwork::FilledBlock& blocks));
                MOCK_METHOD2(removeBlocks, void(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(removeBlocksUpTo, void(uint32_t heightTo));
                MOCK_METHOD0(CleanAll, void());
                MOCK_METHOD2(getBlocks, Future<std::vector<FilledBlock>>(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(getBlock, Future<Option<FilledBlock>>(uint32_t height));
                MOCK_METHOD0(getLastBlockHeader, Future<Option<Block>>());
            };

            class BlockchainDBMock : public db::BlockchainDB {
            public:
                MOCK_METHOD2(AddBlock ,void (uint32_t height, const RawBlock& block));
                MOCK_METHOD2(RemoveBlocks, void(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(RemoveBlocksUpTo, void(uint32_t heightTo));
                MOCK_METHOD0(CleanAll, void());
                MOCK_METHOD2(GetBlocks, Future<std::vector<RawBlock>>(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(GetBlock, Future<Option<RawBlock>>(uint32_t height));
                MOCK_METHOD0(GetLastBlock, Future<Option<RawBlock>>());
            };

            class LoggerSinkMock : public spdlog::sinks::sink {
            public:
                MOCK_METHOD1(log, void(const spdlog::details::log_msg &msg));
                MOCK_METHOD0(flush, void());
            };
        }
    }
}