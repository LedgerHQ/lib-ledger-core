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

            virtual ~ReadOnlyBlockchainDatabase() = 0;
            // Get all blocks with height in [heightFrom, heightTo)
            virtual Future<std::vector<FilledBlock>> getBlocks(int heightFrom, int heightTo) = 0;
            // Return the last block header or genesis block
            virtual Future<Block> getLastBlockHeader() = 0;
        };

        template<typename NetworkType>
        class BlockchainDatabase : public ReadOnlyBlockchainDatabase<NetworkType> {
        public:
            typedef typename NetworkType::FilledBlock FilledBlock;
            typedef typename NetworkType::Block Block;

            virtual ~BlockchainDatabase() = 0;
            virtual Future<Unit> addBlocks(const std::vector<typename FilledBlock>& blocks) = 0;
            // Remove all blocks with height in [heightFrom, heightTo)
            virtual Future<Unit> removeBlocks(int heightFrom, int heightTo) = 0;
            // Remove all blocks with height < heightTo
            virtual Future<Unit> removeBlocksUpTo(int heightTo) = 0;
        };
    };
}