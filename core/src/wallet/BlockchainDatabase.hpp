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
            virtual Future<std::vector<FilledBlock>> getBlocks(int heightFrom, int heightTo) = 0;
            virtual FuturePtr<FilledBlock> getBlock(int height) = 0;
            // Return the last block header or genesis block
            virtual FuturePtr<Block> getLastBlockHeader() = 0;
        };

        template<typename NetworkType>
        class BlockchainDatabase : public ReadOnlyBlockchainDatabase<NetworkType> {
        public:
            typedef typename NetworkType::FilledBlock FilledBlock;
            typedef typename NetworkType::Block Block;

            virtual ~BlockchainDatabase(){};
            virtual void addBlocks(const std::vector<FilledBlock>& blocks) = 0;
            virtual void addBlock(const FilledBlock& block) = 0;
            // Remove all blocks with height in [heightFrom, heightTo)
            virtual void removeBlocks(int heightFrom, int heightTo) = 0;
            // Remove all blocks with height < heightTo
            virtual void removeBlocksUpTo(int heightTo) = 0;
            virtual void CleanAll() = 0;
        };
    };
}