#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {
        template<typename Block>
        class ReadOnlyBlockchainDatabase {
        public:

            virtual ~ReadOnlyBlockchainDatabase() {};
            // Get all blocks with height in [heightFrom, heightTo)
            virtual Future<std::vector<Block>> getBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
            virtual Future<Option<Block>> getBlock(uint32_t height) = 0;
            virtual Future<Option<std::pair<uint32_t, Block>>> getLastBlock() = 0;
            virtual Future<Option<uint32_t>> getLastBlockHeight() = 0;
            // Get last blocks with height in <= height
            virtual Future<Option<std::pair<uint32_t, Block>>> getLastBlockBefore(uint32_t height) = 0;
        };

        template<typename Block>
        class BlockchainDatabase : public ReadOnlyBlockchainDatabase<Block> {
        public:
            virtual ~BlockchainDatabase(){};
            virtual void addBlock(uint32_t height, const Block& block) = 0;
            // Remove all blocks with height in [heightFrom, heightTo)
            virtual void removeBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
            // Remove all blocks with height < heightTo
            virtual void removeBlocksUpTo(uint32_t heightTo) = 0;
            virtual void CleanAll() = 0;
        };
    };
}
