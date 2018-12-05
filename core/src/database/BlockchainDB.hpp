#pragma once

#include <database/DatabaseBlock.hpp>
#include <utils/Option.hpp>
#include <async/Future.hpp>
#include <vector>

namespace ledger {
	namespace core {
		namespace db {
			// Storage interfaces for blockchain
			class ReadOnlyBlockchainDB {
            public:
                virtual ~ReadOnlyBlockchainDB() {};
				// Get all blocks with height in [heightFrom, heightTo)
				virtual Future<std::vector<DatabaseBlock>> GetBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
                virtual Future<Option<DatabaseBlock>> GetBlock(uint32_t height) = 0;
				virtual Future<Option<DatabaseBlockHeader>> GetLastBlockHeader() = 0;
			};

			class BlockchainDB: public ReadOnlyBlockchainDB {
			public:
                virtual ~BlockchainDB() {};
				virtual void AddBlock(const DatabaseBlock& block) = 0;
				// Remove all blocks with height in [heightFrom, heightTo)
				virtual void RemoveBlocks(uint32_t heightFrom, uint32_t heightTo) = 0;
				// Remove all blocks with height < heightTo
				virtual void RemoveBlocksUpTo(uint32_t heightTo) = 0;
                virtual void CleanAll() = 0;
			};
		}
	}
}