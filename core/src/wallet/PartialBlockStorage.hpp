#pragma once

#include <async/Future.hpp>
#include <vector>

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class PartialBlockStorage {
        public:
            typedef typename NetworkType::Block Block;
            typedef typename NetworkType::Transaction Transaction;

            virtual ~PartialBlockStorage() {};
            virtual void addTransaction(const Transaction& transactions) = 0;
            virtual std::vector<Transaction> getTransactions(uint32_t blockHeight) = 0;
            virtual void removeBlock(uint32_t blockHeight) = 0;
        };
    };
}