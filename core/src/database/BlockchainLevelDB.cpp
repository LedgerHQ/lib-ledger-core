#include <database/BlockchainLevelDB.hpp>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <boost/lexical_cast.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace ledger {
    namespace core {
        namespace db {
            BlockchainLevelDB::BlockchainLevelDB(const std::string& pathToDB) {
                leveldb::Options options;
                options.create_if_missing = true;
                leveldb::DB* db;
                handleError(leveldb::DB::Open(options, pathToDB, &db));
                _db = std::shared_ptr<leveldb::DB>(db);
            }

            void BlockchainLevelDB::handleError(const leveldb::Status& status) {
                if (!status.ok()) {
                    throw Exception(api::ErrorCode::UNABLE_TO_OPEN_LEVELDB, status.ToString());
                }
            }

            void BlockchainLevelDB::AddBlock(const DatabaseBlock& block) {
                auto key = serializeKey(block.header.height);
                handleError(_db->Put(leveldb::WriteOptions(), key, serializeValue(block)));
            }

            void BlockchainLevelDB::RemoveBlocks(uint32_t heightFrom, uint32_t heightTo) {
                leveldb::WriteBatch batch;
                for (uint32_t i = heightFrom; i < heightTo; ++i) {
                    batch.Delete(serializeKey(i));
                }
                handleError(_db->Write(leveldb::WriteOptions(), &batch));
            }

            void BlockchainLevelDB::RemoveBlocksUpTo(uint32_t heightTo) {
                leveldb::ReadOptions options;
                options.fill_cache = false;
                leveldb::WriteBatch batch;
                {
                    auto it = std::shared_ptr<leveldb::Iterator>(_db->NewIterator(options));
                    for (it->SeekToFirst(); it->Valid(); it->Next()) {
                        if (deserializeKey(it->key().ToString()) >= heightTo)
                            break;
                        batch.Delete(it->key());
                    }
                }
                handleError(_db->Write(leveldb::WriteOptions(), &batch));
            }

            Future<std::vector<DatabaseBlock>> BlockchainLevelDB::GetBlocks(uint32_t heightFrom, uint32_t heightTo) {
                std::vector<DatabaseBlock> res;
                auto it = std::shared_ptr<leveldb::Iterator>(_db->NewIterator(leveldb::ReadOptions()));
                it->Seek(serializeKey(heightFrom));
                for (; it->Valid(); it->Next()) {
                    if (deserializeKey(it->key().ToString()) >= heightTo)
                        break;
                    res.push_back(getValue(it));
                }
                return Future<std::vector<DatabaseBlock>>::successful(res);
            }

            Future<Option<DatabaseBlock>> BlockchainLevelDB::GetBlock(uint32_t height) {
                std::string value;
                auto status = _db->Get(leveldb::ReadOptions(), serializeKey(height), &value);
                if (status.ok()) {
                    return Future<Option<DatabaseBlock>>::successful(getValue(height, value));
                }
                if (status.IsNotFound()) { // not a error
                    return Future<Option<DatabaseBlock>>::successful(Option<DatabaseBlock>());
                }
                return Future<Option<DatabaseBlock>>::failure(Exception(api::ErrorCode::UNKNOWN, fmt::format("Error during getting block {}: {}", height, status.ToString())));
            }

            Future<Option<DatabaseBlockHeader>> BlockchainLevelDB::GetLastBlockHeader() {
                auto it = std::shared_ptr<leveldb::Iterator>(_db->NewIterator(leveldb::ReadOptions()));
                it->SeekToLast();
                if (!it->Valid())
                    return Future<Option<DatabaseBlockHeader>>::successful(Option<DatabaseBlockHeader>());
                return Future<Option<DatabaseBlockHeader>>::successful(Option<DatabaseBlockHeader>(getValue(it).header));
            }

            void BlockchainLevelDB::CleanAll() {
                RemoveBlocksUpTo(1U << 31U);
            }

            std::string BlockchainLevelDB::serializeKey(uint32_t height) {
                std::string key = fmt::format("{:0>10}", height);
                return key;
            }

            uint32_t BlockchainLevelDB::deserializeKey(const std::string& key) {
                return boost::lexical_cast<uint32_t>(key);
            }

            std::string BlockchainLevelDB::serializeValue(const DatabaseBlock& block) {
                std::stringstream ss;
                {
                    cereal::BinaryOutputArchive oarchive(ss);
                    oarchive(block.header.hash, block.data);
                }
                return ss.str();
            }

            std::pair<std::string, std::vector<uint8_t>> BlockchainLevelDB::deserializeValue(const std::string& value) {
                std::pair<std::string, std::vector<uint8_t>> res;
                std::stringstream ss(value);
                {
                    cereal::BinaryInputArchive iarchive(ss);
                    iarchive(res.first, res.second);
                }
                return res;
            }

            DatabaseBlock BlockchainLevelDB::getValue(uint32_t key, const std::string& hash, const std::vector<uint8_t>& data) {
                DatabaseBlock block;
                block.header.height = key;
                block.header.hash = hash;
                block.data = data;
                return block;
            }

            DatabaseBlock BlockchainLevelDB::getValue(uint32_t key, const std::string& value) {
                auto pr = deserializeValue(value);
                return getValue(key, pr.first, pr.second);
            }

            DatabaseBlock BlockchainLevelDB::getValue(const std::shared_ptr<leveldb::Iterator>& it) {
                return getValue(deserializeKey(it->key().ToString()), it->value().ToString());
            }
        }
    }
}