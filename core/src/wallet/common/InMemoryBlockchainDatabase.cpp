#pragma once

#include <wallet/common/InMemoryBlockchainDatabase.hpp>
#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        namespace common {

            template <typename NetworkType>
            InMemoryBlockchainDatabase<NetworkType>::InMemoryBlockchainDatabase(
                const std::shared_ptr<api::ExecutionContext>& context)
            : _context(context) {
            };

            template <typename NetworkType>
            Future<std::vector<typename NetworkType::FilledBlock>> InMemoryBlockchainDatabase<NetworkType>::getBlocks(uint32_t heightFrom, uint32_t heightTo) {
                std::vector<typename NetworkType::FilledBlock> res;
                std::transform(_blocks.lower_bound(heightFrom), _blocks.lower_bound(heightTo), std::back_inserter(res), [](const decltype(_blocks)::value_type& it) {return it.second; });
                return Future<std::vector<typename NetworkType::FilledBlock>>::successful(res);
            }

            template <typename NetworkType>
            Future<Option<typename NetworkType::FilledBlock>> InMemoryBlockchainDatabase<NetworkType>::getBlock(uint32_t height) {
                std::lock_guard<std::mutex> lock(_lock);
                auto it = _blocks.find(height);
                if (it == _blocks.end())
                    return Future<Option<typename NetworkType::FilledBlock>>::successful(Option<typename NetworkType::FilledBlock>());
                return Future<Option<typename NetworkType::FilledBlock>>::successful(Option<typename NetworkType::FilledBlock>(it->second));
            }

            template <typename NetworkType>
            Future<Option<typename NetworkType::Block>> InMemoryBlockchainDatabase<NetworkType>::getLastBlockHeader() {
                std::lock_guard<std::mutex> lock(_lock);
                if (_blocks.empty()) {
                    return Future<Option<typename NetworkType::Block>>::successful(Option<typename NetworkType::Block>());
                }
                auto lastIt = _blocks.end();
                lastIt--;
                return Future<Option<typename NetworkType::Block>>::successful(Option<typename NetworkType::Block>(lastIt->second.header));
            }

            template <typename NetworkType>
            void InMemoryBlockchainDatabase<NetworkType>::addBlock(const FilledBlock& block) {
                std::lock_guard<std::mutex> lock(_lock);
                _blocks[block.header.height] = block;
            }
            // Remove all blocks with height in [heightFrom, heightTo)
            template <typename NetworkType>
            void InMemoryBlockchainDatabase<NetworkType>::removeBlocks(uint32_t heightFrom, uint32_t heightTo) {
                std::lock_guard<std::mutex> lock(_lock);
                _blocks.erase(_blocks.lower_bound(heightFrom), _blocks.lower_bound(heightTo));
            }
            // Remove all blocks with height < heightTo
            template <typename NetworkType>
            void InMemoryBlockchainDatabase<NetworkType>::removeBlocksUpTo(uint32_t heightTo) {
                std::lock_guard<std::mutex> lock(_lock);
                _blocks.erase(_blocks.begin(), _blocks.lower_bound(heightTo));
            }
            template <typename NetworkType>
            void InMemoryBlockchainDatabase<NetworkType>::CleanAll() {
                std::lock_guard<std::mutex> lock(_lock);
                _blocks.clear();
            }
        }
    };
}

template class ledger::core::common::InMemoryBlockchainDatabase<ledger::core::BitcoinLikeNetwork>;
template class std::shared_ptr<ledger::core::common::InMemoryBlockchainDatabase<ledger::core::BitcoinLikeNetwork>>;