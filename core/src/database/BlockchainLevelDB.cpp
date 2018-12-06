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

            void BlockchainLevelDB::AddBlock(uint32_t height, const RawBlock& block) {
                auto key = serializeKey(height);
                leveldb::Slice slice(reinterpret_cast<const char*>(block.data()), block.size());
                handleError(_db->Put(leveldb::WriteOptions(), key, slice));
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

            Future<std::vector<BlockchainDB::RawBlockWithHeight>> BlockchainLevelDB::GetBlocks(uint32_t heightFrom, uint32_t heightTo) {
                std::vector<RawBlockWithHeight> res;
                auto it = std::shared_ptr<leveldb::Iterator>(_db->NewIterator(leveldb::ReadOptions()));
                it->Seek(serializeKey(heightFrom));
                for (; it->Valid(); it->Next()) {
                    uint32_t height = deserializeKey(it->key());
                    if (height >= heightTo)
                        break;
                    res.push_back(std::make_pair(height, sliceToBlock(it->value())));
                }
                return Future<std::vector<RawBlockWithHeight>>::successful(res);
            }

            Future<Option<BlockchainDB::RawBlock>> BlockchainLevelDB::GetBlock(uint32_t height) {
                std::string value;
                auto status = _db->Get(leveldb::ReadOptions(), serializeKey(height), &value);
                if (status.ok()) {
                    return Future<Option<RawBlock>>::successful(RawBlock());
                }
                if (status.IsNotFound()) { // not a error
                    return Future<Option<RawBlock>>::successful(Option<RawBlock>());
                }
                return Future<Option<RawBlock>>::failure(Exception(api::ErrorCode::UNKNOWN, fmt::format("Error during getting block {}: {}", height, status.ToString())));
            }

            Future<Option<BlockchainDB::RawBlockWithHeight>> BlockchainLevelDB::GetLastBlock() {
                auto it = std::shared_ptr<leveldb::Iterator>(_db->NewIterator(leveldb::ReadOptions()));
                it->SeekToLast();
                if (!it->Valid())
                    return Future<Option<RawBlockWithHeight>>::successful(Option<RawBlockWithHeight>());
                return Future<Option<RawBlockWithHeight>>::successful(
                    Option<RawBlockWithHeight>(std::make_pair(deserializeKey(it->key()), sliceToBlock(it->value()))));
            }

            BlockchainDB::RawBlock BlockchainLevelDB::sliceToBlock(const leveldb::Slice& value) {
                return RawBlock(value.data(), value.data() + value.size());
            }

            void BlockchainLevelDB::CleanAll() {
                RemoveBlocksUpTo(1U << 31U);
            }

            std::string BlockchainLevelDB::serializeKey(uint32_t height) {
                std::string key = fmt::format("{:0>10}", height);
                return key;
            }

            uint32_t BlockchainLevelDB::deserializeKey(const leveldb::Slice& key) {
                return boost::lexical_cast<uint32_t>(key.data(), key.size());
            }
        }
    }
}