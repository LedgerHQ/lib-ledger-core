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
				virtual Future<std::vector<DatabaseBlock>> GetBlocks(int heightFrom, int heightTo) = 0;
				virtual Future<Option<DatabaseBlockHeader>> GetLastBlockHeader() = 0;
			};

			class BlockchainDB: public ReadOnlyBlockchainDB {
			public:
                virtual ~BlockchainDB() {};
				virtual Future<Unit> AddBlocks(const std::vector<DatabaseBlock>& blocks) = 0;
				// Remove all blocks with height in [heightFrom, heightTo)
				virtual Future<Unit> RemoveBlocks(int heightFrom, int heightTo) = 0;
				// Remove all blocks with height < heightTo
				virtual Future<Unit> RemoveBlocksUpTo(int heightTo) = 0;
			};
		}
	}
}