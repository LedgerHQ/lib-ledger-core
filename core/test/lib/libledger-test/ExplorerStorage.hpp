#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {
        namespace test {
            // This class simulate the work of Explorer for unit tests
            class ExplorerStorage {
            public:
                void addTransaction(const std::string& jsonTransaction);

                void removeTransaction(const std::string& hash);

                std::vector<std::string> getTransactions(
                    const std::vector<std::string>& addresses,
                    const std::string& blockHash);

                std::string getLastBlock();
            private:
                std::vector<std::pair<BitcoinLikeBlockchainExplorerTransaction, std::string>> _transactions;
                std::vector<std::pair<BitcoinLikeBlockchainExplorerTransaction, std::string>> _memPool;
            };
        }
    }
}