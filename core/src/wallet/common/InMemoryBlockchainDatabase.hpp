#pragma once

#include <async/Future.hpp>
#include <vector>
#include <mutex>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        namespace api {
            class ExecutionContext;
        };
        namespace common {
            template<typename Block>
            class InMemoryBlockchainDatabase : public BlockchainDatabase<Block> {
            public:
                InMemoryBlockchainDatabase(
                    const std::shared_ptr<api::ExecutionContext>& context) : _context(context) {
                };

                Future<std::vector<Block>> getBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    std::vector<Block> res;
                    std::transform(_blocks.lower_bound(heightFrom), _blocks.lower_bound(heightTo), std::back_inserter(res), [](const std::pair<uint32_t, Block>& it) {return it.second; });
                    return Future<std::vector<Block>>::successful(res);
                }

                Future<Option<Block>> getBlock(uint32_t height) override {
                    std::lock_guard<std::mutex> lock(_lock);
                    auto it = _blocks.find(height);
                    if (it == _blocks.end())
                        return Future<Option<Block>>::successful(Option<Block>());
                    return Future<Option<Block>>::successful(Option<Block>(it->second));
                }

                Future<Option<std::pair<uint32_t, Block>>> getLastBlock() override {
                    std::lock_guard<std::mutex> lock(_lock);
                    if (_blocks.empty()) {
                        return Future<Option<std::pair<uint32_t, Block>>>::successful(Option<std::pair<uint32_t, Block>>());
                    }
                    auto lastIt = _blocks.end();
                    lastIt--;
                    return Future<Option<std::pair<uint32_t, Block>>>::successful(Option<std::pair<uint32_t, Block>>(*lastIt));
                }

                Future<Option<uint32_t>> getLastBlockHeight() override {
                    std::lock_guard<std::mutex> lock(_lock);

                    if (_blocks.empty()) {
                        return Future<Option<uint32_t>>::successful(Option<uint32_t>());
                    }

                    auto lastIt = _blocks.end();
                    lastIt--;
                    return Future<Option<uint32_t>>::successful(Option<uint32_t>(lastIt->first));
                }

                void addBlock(uint32_t height, const Block& block) override {
                    std::lock_guard<std::mutex> lock(_lock);
                    _blocks[height] = block;
                }

                void removeBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    std::lock_guard<std::mutex> lock(_lock);
                    _blocks.erase(_blocks.lower_bound(heightFrom), _blocks.lower_bound(heightTo));
                }

                void removeBlocksUpTo(uint32_t heightTo) override {
                    std::lock_guard<std::mutex> lock(_lock);
                    _blocks.erase(_blocks.begin(), _blocks.lower_bound(heightTo));
                }
                void CleanAll() override {
                    std::lock_guard<std::mutex> lock(_lock);
                    _blocks.clear();
                }
            private:
                std::shared_ptr<api::ExecutionContext> _context;
                std::map<uint32_t, Block> _blocks;
                std::mutex _lock;
            };
        }
    };
}


