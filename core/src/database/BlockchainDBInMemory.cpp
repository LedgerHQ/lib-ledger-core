#pragma once
#include <iterator> 
#include <database/BlockchainDBInMemory.hpp>

namespace ledger {
    namespace core {
        namespace db {

            Future<std::vector<BlockchainDB::RawBlock>> BlockchainDBInMemory::GetBlocks(uint32_t heightFrom, uint32_t heightTo) {
                std::lock_guard<std::mutex> lock(_lock);
                std::vector<RawBlock> res;
                std::transform(_db.lower_bound(heightFrom), _db.lower_bound(heightTo), std::back_inserter(res), [](const decltype(_db)::value_type& it) {return it.second; });
                return Future<std::vector<RawBlock>>::successful(res);
            }

            Future<Option<BlockchainDB::RawBlock>> BlockchainDBInMemory::GetBlock(uint32_t height) {
                std::lock_guard<std::mutex> lock(_lock);
                auto it = _db.find(height);
                if (it == _db.end())
                    return Future<Option<RawBlock>>::successful(Option<RawBlock>());
                return Future<Option<RawBlock>>::successful(Option<RawBlock>(it->second));
            }

            Future<Option<BlockchainDB::RawBlock>> BlockchainDBInMemory::GetLastBlock() {
                std::lock_guard<std::mutex> lock(_lock);
                if (_db.empty()) {
                    return Future<Option<RawBlock>>::successful(Option<RawBlock>());
                }
                auto lastIt = _db.end();
                lastIt--;
                return Future<Option<RawBlock>>::successful(Option<RawBlock>(lastIt->second));
            }

            void BlockchainDBInMemory::AddBlock(uint32_t height, const RawBlock& block) {
                std::lock_guard<std::mutex> lock(_lock);
                _db[height] = block;
            }

            void BlockchainDBInMemory::RemoveBlocks(uint32_t heightFrom, uint32_t heightTo) {
                std::lock_guard<std::mutex> lock(_lock);
                _db.erase(_db.lower_bound(heightFrom), _db.lower_bound(heightTo));
            }

            void BlockchainDBInMemory::RemoveBlocksUpTo(uint32_t heightTo) {
                std::lock_guard<std::mutex> lock(_lock);
                _db.erase(_db.begin(), _db.lower_bound(heightTo));
            }

            void BlockchainDBInMemory::CleanAll() {
                std::lock_guard<std::mutex> lock(_lock);
                _db.clear();
            }
        }
    }
}