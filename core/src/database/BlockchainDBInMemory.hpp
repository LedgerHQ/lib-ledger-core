#pragma once

#include <database/BlockchainDB.hpp>
#include <mutex>
#include <map>

namespace ledger {
    namespace core {
        namespace db {

            class BlockchainDBInMemory : public BlockchainDB {
            public:
                Future<std::vector<RawBlock>> GetBlocks(uint32_t heightFrom, uint32_t heightTo) override;
                Future<Option<RawBlock>> GetBlock(uint32_t height) override;
                Future<Option<std::pair<uint32_t, RawBlock>>> GetLastBlock() override;
                void AddBlock(uint32_t height, const RawBlock& block) override;
                void RemoveBlocks(uint32_t heightFrom, uint32_t heightTo) override;
                void RemoveBlocksUpTo(uint32_t heightTo) override;
                void CleanAll() override;
            private:
                std::map<uint32_t, RawBlock> _db;
                std::mutex _lock;
            };
        }
    }
}