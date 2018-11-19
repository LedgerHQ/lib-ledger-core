#pragma once

#include <async/Future.hpp>
#include <vector>

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class PartialBlockStorage {
        public:
            typedef NetworkType::Block Block;
            typedef NetworkType::Transaction Transaction;

            virtual Future<Unit> AddTransactions(const Block& block, const std::vector<Transaction>& transactions) = 0;
            virtual Future<std::vector<Transaction>> GetTransaction(const Block& block) = 0;
            virtual Future<Unit> Clear() = 0;
        };
    };
}