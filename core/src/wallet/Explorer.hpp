#pragma once

#include <utility>
#include <vector>
#include <utils/Option.hpp>
#include <async/Future.hpp>
#include <collections/Bytes.hpp>

namespace ledger {
    namespace core {
        // An iterface for blockchain explorer API v2
        template<typename Network>
        class ExplorerV2 {
        public:
            typedef typename Network::Transaction Transaction;
            typedef typename Network::Block Block;
            typedef std::pair<std::vector<typename Transaction>, bool> TransactionBulk;

            virtual Future<void *> startSession() = 0;
            virtual Future<Unit> killSession(void *session) = 0;

            virtual FuturePtr<TransactionBulk> getTransactions(
                const std::vector<std::string>& addresses,
                Option<std::string> fromBlockHash = Option<std::string>(),
                Option<void*> session = Option<void *>()
            ) = 0;

            virtual FuturePtr<Block> getCurrentBlock() = 0;
            virtual Future<Bytes> getRawTransaction(const std::string& transactionHash) = 0;
            virtual FuturePtr<Transaction> getTransactionByHash(const std::string& transactionHash) = 0;
            virtual Future<std::string> pushTransaction(const std::vector<uint8_t>& transaction) = 0;
            virtual Future<int64_t> getTimestamp() = 0;
        };
    };
};