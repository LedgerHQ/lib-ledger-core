#pragma once
#include <gmock/gmock.h>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        namespace tests {
            class ExplorerMock : public ExplorerV2<BitcoinLikeNetwork> {
            public:
                MOCK_METHOD0(startSession, Future<void *>());
                MOCK_METHOD1(killSession, Future<Unit>(void *session));
                MOCK_METHOD3(getTransactions, FuturePtr<TransactionBulk>(const std::vector<std::string>& addresses, Option<std::string> fromBlockHash, Option<void*> session));
                MOCK_METHOD0(getCurrentBlock, FuturePtr<Block>());
                MOCK_METHOD1(getRawTransaction, Future<Bytes>(const std::string& transactionHash));
                MOCK_METHOD1(getTransactionByHash, FuturePtr<Transaction>(const std::string& transactionHash));
                MOCK_METHOD1(pushTransaction, Future<std::string>(const std::vector<uint8_t>& transaction));
                MOCK_METHOD0(getTimestamp, Future<int64_t>());
            };

            class KeychainMock : public Keychain<BitcoinLikeNetwork> {
            public:
                MOCK_METHOD0(getNumberOfUsedAddresses, uint32_t());
                MOCK_METHOD2(getAddresses, std::vector<std::string>(uint32_t startIndex, uint32_t count));
                MOCK_METHOD1(markAsUsed, void(std::string& address));
            };

            class BlocksDBMock : public BlockchainDatabase<BitcoinLikeNetwork> {
            public:
                MOCK_METHOD1(addBlocks, Future<Unit>(const std::vector<BitcoinLikeNetwork::FilledBlock>& blocks));
                MOCK_METHOD2(removeBlocks, Future<Unit>(int heightFrom, int heightTo));
                MOCK_METHOD1(removeBlocksUpTo, Future<Unit>(int heightTo));
                MOCK_METHOD2(getBlocks, FuturePtr<std::vector<FilledBlock>>(int heightFrom, int heightTo));
                MOCK_METHOD0(getLastBlockHeader, FuturePtr<Block>());
            };
        }
    }
}