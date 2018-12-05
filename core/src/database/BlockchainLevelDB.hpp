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
                void AddBlock(const DatabaseBlock& block) override;
                void RemoveBlocks(uint32_t heightFrom, uint32_t heightTo) override;
                void RemoveBlocksUpTo(uint32_t heightTo) override;
                void CleanAll() override;
                Future<std::vector<DatabaseBlock>> GetBlocks(uint32_t heightFrom, uint32_t heightTo) override;
                Future<Option<DatabaseBlock>> GetBlock(uint32_t height) override;
                Future<Option<DatabaseBlockHeader>> GetLastBlockHeader() override;
            private:
                void handleError(const leveldb::Status& status);
                std::string serializeKey(uint32_t height);
                uint32_t deserializeKey(const std::string& key); 
                std::string serializeValue(const DatabaseBlock& block);
                std::pair<std::string, std::vector<uint8_t>> deserializeValue(const std::string& value);
                DatabaseBlock getValue(uint32_t key, const std::string& hash, const std::vector<uint8_t>& data);
                DatabaseBlock getValue(uint32_t key, const std::string& value);
                DatabaseBlock getValue(const std::shared_ptr<leveldb::Iterator>& it);
            private:
                std::shared_ptr<leveldb::DB> _db;
            };
        }
    }
}