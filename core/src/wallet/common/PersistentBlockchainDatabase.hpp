#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <database/BlockchainDB.hpp>

namespace ledger {
    namespace core {
        namespace common {
            template<typename NetworkType>
            class PersistentBlockchainDatabase : public BlockchainDatabase<NetworkType> {
            public:
                typedef typename NetworkType::FilledBlock FilledBlock;
                typedef typename NetworkType::Block Block;

                PersistentBlockchainDatabase(
                    std::shared_ptr<api::ExecutionContext>& context,
                    std::shared_ptr<db::BlockchainDB>& persistentDB)
                : _context(context)
                , _persistentDB(persistentDB) {
                };

                Future<std::vector<FilledBlock>> getBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    return _persistentDB->GetBlocks(heightFrom, heightTo)
                        .map(_context, [](const std::vector<std::vector<>>& blocks) {
                        std::vector<FilledBlock> res;
                        for (auto& rawBlock : blocks) {
                            res.push_back();
                        }
                        return res;
                    });
                }
                FuturePtr<FilledBlock> getBlock(uint32_t height) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING);
                }
                // Return the last block header or genesis block
                FuturePtr<Block> getLastBlockHeader() override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING);
                }
                void addBlocks(const std::vector<FilledBlock>& blocks) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING);
                }
                void addBlock(const FilledBlock& block) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING);
                }
                // Remove all blocks with height in [heightFrom, heightTo)
                void removeBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING);
                }
                // Remove all blocks with height < heightTo
                void removeBlocksUpTo(uint32_t heightTo) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING);
                }
                void CleanAll() override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING);
                }
            private:
                std::shared_ptr<api::ExecutionContext> _context;
                std::shared_ptr<db::BlockchainDB> _persistentDB;
            };
        }
    };
}