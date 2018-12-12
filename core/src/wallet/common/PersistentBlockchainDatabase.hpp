#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <database/BlockchainDB.hpp>
#include <cereal/archives/binary.hpp>

namespace ledger {
    namespace core {
        namespace common {
            template<typename NetworkType>
            class PersistentBlockchainDatabase : public BlockchainDatabase<NetworkType> {
            public:
                typedef typename NetworkType::FilledBlock FilledBlock;
                typedef typename NetworkType::Block Block;
                typedef typename db::BlockchainDB::RawBlock RawBlock;

                PersistentBlockchainDatabase(
                    const std::shared_ptr<api::ExecutionContext>& context,
                    const std::shared_ptr<db::BlockchainDB>& persistentDB)
                : _context(context)
                , _persistentDB(persistentDB) {
                };

                Future<std::vector<FilledBlock>> getBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    return _persistentDB->GetBlocks(heightFrom, heightTo)
                        .map<std::vector<FilledBlock>>(_context, [](const std::vector<RawBlock>& blocks) {
                        std::vector<FilledBlock> res;
                        for (auto& rawBlock : blocks) {
                            FilledBlock fb;
                            deserialize(rawBlock, fb);
                            res.push_back(fb);
                        }
                        return res;
                    });
                }

                Future<Option<FilledBlock>> getBlock(uint32_t height) override {
                    return _persistentDB->GetBlock(height)
                        .map<Option<FilledBlock>>(_context, [](const Option<RawBlock>& rawBlock) {
                        return rawBlock.map<FilledBlock>([](const RawBlock& rawBlock) {
                            FilledBlock fb;
                            deserialize(rawBlock, fb);
                            return fb;});
                    });
                }

                // Return the last block header or genesis block
                Future<Option<Block>> getLastBlockHeader() override {
                    return _persistentDB->GetLastBlock()
                        .map<Option<Block>>(_context, [](const Option<RawBlock>& rawBlock) {
                        return rawBlock.map<Block>([](const RawBlock& block) {
                            FilledBlock filledBlock;
                            deserialize(block, filledBlock);
                            return filledBlock.header;
                        });
                    });
                }

                void addBlock(const FilledBlock& block) override {
                    RawBlock rawBlock = serialize(block);
                    _persistentDB->AddBlock(block.header.height, rawBlock);
                }
                // Remove all blocks with height in [heightFrom, heightTo)
                void removeBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    _persistentDB->RemoveBlocks(heightFrom, heightTo);
                }
                // Remove all blocks with height < heightTo
                void removeBlocksUpTo(uint32_t heightTo) override {
                    _persistentDB->RemoveBlocksUpTo(heightTo);
                }
                void CleanAll() override {
                    _persistentDB->CleanAll();
                }
            private:
                static std::vector<uint8_t> serialize(const FilledBlock& filledBlock) {
                    std::ostringstream  ss;
                    {
                        cereal::BinaryOutputArchive oarchive(ss);
                        oarchive(filledBlock);
                    }
                    auto data = ss.str();
                    return std::vector<uint8_t>(data.begin(), data.end());
                }

                static void deserialize(const std::vector<uint8_t>& buf, FilledBlock& filledBlock) {
                    std::istringstream  ss(std::string(buf.begin(), buf.end()));
                    {
                        cereal::BinaryInputArchive iarchive(ss);
                        iarchive(filledBlock);
                    }
                }

            private:
                std::shared_ptr<api::ExecutionContext> _context;
                std::shared_ptr<db::BlockchainDB> _persistentDB;
            };
        }
    };
}