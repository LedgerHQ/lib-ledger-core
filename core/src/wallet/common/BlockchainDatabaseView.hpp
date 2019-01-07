#pragma once
#include <wallet/BlockchainDatabase.hpp>
#include <async/Future.hpp>

namespace ledger {
    namespace core {
        namespace common {
            template<typename Block>
            class BlockchainDatabaseView : public ReadOnlyBlockchainDatabase<Block> {
            public:
                BlockchainDatabaseView(
                    uint32_t limitingHeight,
                    ReadOnlyBlockchainDatabase<Block>& db)
                    : _db(db) {
                }

                Future<std::vector<Block>> getBlocks(uint32_t heightFrom, uint32_t heightTo) override {
                    return _db->getBlocks(std::min(heightFrom, _limitingHeight), std::min(heightTo, _limitingHeight + 1));
                }

                Future<Option<Block>> getBlock(uint32_t height) override {
                    if (height > _limitingHeight) {
                        return Future<Option<Block>>::successful(Option<Block>());
                    }
                    return _db->getBlock(height);
                }

                Future<Option<std::pair<uint32_t, Block>>> getLastBlock() override {
                    return _db->getLastBlockBefore(_limitingHeight);
                }
                Future<Option<std::pair<uint32_t, Block>>> getLastBlockBefore(uint32_t height) override {
                    return _db->getLastBlockBefore(std::min(height, _limitingHeight));
                }
            private:
                std::shared_ptr<ReadOnlyBlockchainDatabase<Block>> _db;
                uint32_t _limitingHeight;
            };
        }
    }
}