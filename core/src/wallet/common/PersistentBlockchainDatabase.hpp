#pragma once

#include <async/Future.hpp>
#include <vector>
#include <cereal/archives/binary.hpp>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        namespace api {
            class ExecutionContext;
        };
        namespace db {
            class BlockchainDB;
        };
        namespace common {
            using RawBlock = db::BlockchainDB::RawBlock;

            template<typename BlockType>
            std::vector<uint8_t> serialize(const BlockType& block) {
                std::ostringstream  ss;
                {
                    cereal::BinaryOutputArchive oarchive(ss);
                    oarchive(block);
                }
                auto data = ss.str();
                return std::vector<uint8_t>(data.begin(), data.end());
            }

            template <typename BlockType>
            void deserialize(const std::vector<uint8_t>& buf, BlockType& block) {
                std::istringstream  ss(std::string(buf.begin(), buf.end()));
                {
                    cereal::BinaryInputArchive iarchive(ss);
                    iarchive(block);
                }
            }

            template<typename Block>
            class PersistentBlockchainDatabase : public BlockchainDatabase<Block> {
            public:
                PersistentBlockchainDatabase(
                    const std::shared_ptr<api::ExecutionContext>& context,
                    const std::shared_ptr<db::BlockchainDB>& persistentDB) 
                    : _context(context)
                    , _persistentDB(persistentDB) {
                };

                Future<std::vector<Block>> getBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    return _persistentDB->GetBlocks(heightFrom, heightTo)
                        .map<std::vector<Block>>(_context, [](const std::vector<RawBlock>& blocks) {
                        std::vector<Block> res;
                        for (auto& rawBlock : blocks) {
                            Block fb;
                            deserialize<Block>(rawBlock, fb);
                            res.push_back(fb);
                        }
                        return res;
                    });
                }

                Future<Option<Block>> getBlock(uint32_t height) override {
                    return _persistentDB->GetBlock(height)
                        .map<Option<Block>>(_context, [](const Option<RawBlock>& rawBlock) {
                        return rawBlock.map<Block>([](const RawBlock& rawBlock) {
                            Block fb;
                            deserialize<Block>(rawBlock, fb);
                            return fb; });
                    });
                }

                Future<Option<std::pair<uint32_t, Block>>> getLastBlock() override {
                    return _persistentDB->GetLastBlock()
                        .map<Option<std::pair<uint32_t, Block>>>(_context, [](const Option<std::pair<uint32_t, RawBlock>>& rawBlock) {
                        return rawBlock.map<std::pair<uint32_t, Block>>([](const std::pair<uint32_t, RawBlock>& rawBlock) {
                            Block block;
                            deserialize<Block>(rawBlock.second, block);
                            return std::make_pair(rawBlock.first, block);
                        });
                    });
                }

                Future<Option<std::pair<uint32_t, Block>>> getLastBlockBefore(uint32_t height) override {
                    return _persistentDB->GetLastBlockBefore(height)
                        .map<Option<std::pair<uint32_t, Block>>>(_context, [](const Option<std::pair<uint32_t, RawBlock>>& rawBlock) {
                        return rawBlock.map<std::pair<uint32_t, Block>>([](const std::pair<uint32_t, RawBlock>& rawBlock) {
                            Block block;
                            deserialize<Block>(rawBlock.second, block);
                            return std::make_pair(rawBlock.first, block);
                        });
                    });
                }

                Future<Option<uint32_t>> getLastBlockHeight() override {
                    return _persistentDB->GetLastBlock()
                        .map<Option<uint32_t>>(_context, [](const Option<std::pair<uint32_t, RawBlock>>& rawBlock) {
                        return rawBlock.map<uint32_t>([](const std::pair<uint32_t, RawBlock>& rawBlock) {
                            return rawBlock.first;
                        });
                    });
                }

                void addBlock(uint32_t height, const Block& block) override {
                    RawBlock rawBlock = serialize<Block>(block);
                    _persistentDB->AddBlock(height, rawBlock);
                }

                void removeBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    _persistentDB->RemoveBlocks(heightFrom, heightTo);
                }

                void removeBlocksUpTo(uint32_t heightTo) override {
                    _persistentDB->RemoveBlocksUpTo(heightTo);
                }

                void CleanAll() override {
                    _persistentDB->CleanAll();
                }
            private:
                std::shared_ptr<api::ExecutionContext> _context;
                std::shared_ptr<db::BlockchainDB> _persistentDB;
            };
        }
    };
}
