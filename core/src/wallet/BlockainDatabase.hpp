#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class ReadOnlyBlockchainDatabase {
            typedef NetworkType::FilledBlock FilledBlock;
            typedef NetworkType::Block Block;

            virtual ~ReadOnlyBlockchainDatabase() = 0;
            // Get all blocks with height in [heightFrom, heightTo)
            virtual Future<std::vector<FilledBlock>> GetBlocks(int heightFrom, int heightTo) = 0;
            virtual Future<Option<Block>> GetLastBlockHeader() = 0;
        };

        template<typename NetworkType>
        class BlockchainDatabase : public ReadOnlyBlockchainDatabase<NetworkType> {
        public:
            typedef NetworkType::FilledBlock FilledBlock;
            typedef NetworkType::Block Block;

            virtual ~BlockchainDatabase() = 0;
            virtual Future<Unit> AddBlocks(const std::vector<FilledBlock>& blocks) = 0;
            // Remove all blocks with height in [heightFrom, heightTo)
            virtual Future<Unit> RemoveBlocks(int heightFrom, int heightTo) = 0;
            // Remove all blocks with height < heightTo
            virtual Future<Unit> RemoveBlocksUpTo(int heightTo) = 0;
        };
    };
}