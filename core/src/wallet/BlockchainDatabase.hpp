#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class ReadOnlyBlockchainDatabase {
        public:
            typedef typename NetworkType::FilledBlock FilledBlock;
            typedef typename NetworkType::Block Block;

            virtual ~ReadOnlyBlockchainDatabase() {};
            // Get all blocks with height in [heightFrom, heightTo)
            virtual Future<std::vector<FilledBlock>> getBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
            virtual Future<Option<FilledBlock>> getBlock(uint32_t height) = 0;
            // Return the last block header or genesis block
            virtual Future<Option<Block>> getLastBlockHeader() = 0;
        };

        template<typename NetworkType>
        class BlockchainDatabase : public ReadOnlyBlockchainDatabase<NetworkType> {
        public:
            typedef typename NetworkType::FilledBlock FilledBlock;
            typedef typename NetworkType::Block Block;

            virtual ~BlockchainDatabase(){};
            virtual void addBlock(const FilledBlock& block) = 0;
            // Remove all blocks with height in [heightFrom, heightTo)
            virtual void removeBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
            // Remove all blocks with height < heightTo
            virtual void removeBlocksUpTo(uint32_t heightTo) = 0;
            virtual void CleanAll() = 0;
        };
    };
}