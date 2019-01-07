#pragma once

#include <database/BlockchainDB.hpp>
#include <memory>
#include <string>

namespace leveldb { 
    class DB; 
    class Status;
    class Slice;
    class Iterator;
};

namespace ledger {
    namespace core {
        namespace db {
            class BlockchainLevelDB : public BlockchainDB {
            public:
                BlockchainLevelDB(const std::string& pathToDB);
                void AddBlock(uint32_t height, const RawBlock& block) override;
                void RemoveBlocks(uint32_t heightFrom, uint32_t heightTo) override;
                void RemoveBlocksUpTo(uint32_t heightTo) override;
                void CleanAll() override;
                Future<std::vector<RawBlock>> GetBlocks(uint32_t heightFrom, uint32_t heightTo) override;
                Future<Option<RawBlock>> GetBlock(uint32_t height) override;
                Future<Option<std::pair<uint32_t, RawBlock>>> GetLastBlock() override;
                Future<Option<std::pair<uint32_t, RawBlock>>> GetLastBlockBefore(uint32_t height) override;
            private:
                void handleError(const leveldb::Status& status);
                std::string serializeKey(uint32_t height);
                uint32_t deserializeKey(const leveldb::Slice& key);
                RawBlock sliceToBlock(const leveldb::Slice& value);
            private:
                std::shared_ptr<leveldb::DB> _db;
            };
        }
    }
}