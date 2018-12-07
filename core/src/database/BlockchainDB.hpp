#pragma once

#include <utils/Option.hpp>
#include <async/Future.hpp>
#include <vector>

namespace ledger {
	namespace core {
		namespace db {
			// Storage interfaces for blockchain
			class ReadOnlyBlockchainDB {
            public:
                typedef std::vector<uint8_t> RawBlock;
                
                virtual ~ReadOnlyBlockchainDB() {};
				// Get all blocks with height in [heightFrom, heightTo)
				virtual Future<std::vector<RawBlock>> GetBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
                virtual Future<Option<RawBlock>> GetBlock(uint32_t height) = 0;
				virtual Future<Option<RawBlock>> GetLastBlock() = 0;
			};

			class BlockchainDB: public ReadOnlyBlockchainDB {
			public:
                virtual ~BlockchainDB() {};
				virtual void AddBlock(uint32_t height, const RawBlock& block) = 0;
				// Remove all blocks with height in [heightFrom, heightTo)
				virtual void RemoveBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
				// Remove all blocks with height < heightTo
				virtual void RemoveBlocksUpTo(uint32_t heightTo) = 0;
                virtual void CleanAll() = 0;
			};
		}
	}
}