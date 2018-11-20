#pragma once

#include <vector>
#include <mutex>
#include <async/Future.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/AddressesSources.hpp>
#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>

namespace ledger {
    namespace core {
        template<>
        class AddressSources<BitcoinLikeNetwork> {
        public:
            AddressSources(const std::shared_ptr<BitcoinLikeKeychain>& bitcoinKeychain, uint32_t batchSize)
                : _keychain(bitcoinKeychain), _batchSize(batchSize) {
                addNewBatch();
            }

            uint32_t GetNumberOfBatches() {
                std::lock_guard<std::mutex> lock(_lock);
                return _batches.size();
            }
            
            void MarkBatchAsUsed(uint32_t batchIndex) {
                std::lock_guard<std::mutex> lock(_lock);
                if (batchIndex == _batches.size() - 1)
                    addNewBatch();
            }

            std::vector<std::string> GetBatch(uint32_t batchIndex)
            {
                std::lock_guard<std::mutex> lock(_lock);
                return _batches[batchIndex];
            }
        private:
            void addNewBatch()
            {
            }
        private:
            std::mutex _lock;
            std::shared_ptr<BitcoinLikeKeychain> _keychain;
            std::vector<std::vector<std::string>> _batches;
            uint32_t _batchSize;
        };
    }
}