#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/PartialBlockStorage.hpp>
#include <unordered_map>

namespace ledger {
    namespace core {
        namespace common {
            template<typename Transaction>
            class InMemoryPartialBlocksDB : public PartialBlockStorage<Transaction> {
            public:
                void addTransaction(const Transaction& transaction) override {
                    if (!transaction.block.hasValue()) {
                        return;
                    }
                    uint32_t height = transaction.block.getValue().height;
                    _transactions[height][transaction.hash] = transaction;
                }

                std::vector<Transaction> getTransactions(uint32_t blockHeight) override {
                    std::vector<Transaction> res;
                    auto it = _transactions.find(blockHeight);
                    if (it == _transactions.end())
                        return res;
                    for (auto & pr : it->second) {
                        res.push_back(pr.second);
                    }
                    return res;
                }

                void removeBlock(uint32_t blockHeight) override {
                    _transactions.erase(blockHeight);
                }

            private:
                std::unordered_map<uint32_t, std::map<std::string, Transaction>> _transactions;
                std::mutex _lock;
            };
        }
    };
}