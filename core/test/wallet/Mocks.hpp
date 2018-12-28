#pragma once
#include <database/BlockchainDB.hpp>
#include <gmock/gmock.h>
#include <spdlog/sinks/sink.h>
#include <wallet/BalanceService.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/bitcoin/UTXOSource.hpp>
#include <wallet/bitcoin/UTXOService.hpp>

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

            class BlocksDBMock : public BlockchainDatabase<BitcoinLikeNetwork::FilledBlock> {
            public:
                MOCK_METHOD2(addBlock, void(uint32_t height, const BitcoinLikeNetwork::FilledBlock& blocks));
                MOCK_METHOD2(removeBlocks, void(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(removeBlocksUpTo, void(uint32_t heightTo));
                MOCK_METHOD0(CleanAll, void());
                MOCK_METHOD2(getBlocks, Future<std::vector<BitcoinLikeNetwork::FilledBlock>>(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(getBlock, Future<Option<BitcoinLikeNetwork::FilledBlock>>(uint32_t height));
                MOCK_METHOD0(getLastBlock, Future<Option<std::pair<uint32_t, BitcoinLikeNetwork::FilledBlock>>>());
            };

            class BlockchainDBMock : public db::BlockchainDB {
            public:
                MOCK_METHOD2(AddBlock ,void (uint32_t height, const RawBlock& block));
                MOCK_METHOD2(RemoveBlocks, void(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(RemoveBlocksUpTo, void(uint32_t heightTo));
                MOCK_METHOD0(CleanAll, void());
                MOCK_METHOD2(GetBlocks, Future<std::vector<RawBlock>>(uint32_t heightFrom, uint32_t heightTo));
                MOCK_METHOD1(GetBlock, Future<Option<RawBlock>>(uint32_t height));
                MOCK_METHOD0(GetLastBlock, Future<Option<std::pair<uint32_t, RawBlock>>>());
            };

            class LoggerSinkMock : public spdlog::sinks::sink {
            public:
                MOCK_METHOD1(log, void(const spdlog::details::log_msg &msg));
                MOCK_METHOD0(flush, void());
            };

            class UTXOSourceMock : public bitcoin::UTXOSource {
            public:
                MOCK_METHOD1(getUTXOs, Future<bitcoin::UTXOSourceList> (std::shared_ptr<api::ExecutionContext> ctx));
            };

            class UTXOServiceMock : public bitcoin::UTXOService {
            public:
                MOCK_METHOD0(getUTXOs, Future<std::map<bitcoin::UTXOKey, bitcoin::UTXOValue>>());
            };

            class KeychainRegistryMock : public KeychainRegistry {
            public:
                MOCK_METHOD1(containsAddress, bool(const std::string&));
            };
        }
    }
}