#ifndef LEDGER_CORE_DB_BLOCKCHAINDB_HPP
#define LEDGER_CORE_DB_BLOCKCHAINDB_HPP

#include <database/DatabaseBlock.hpp>
#include <utils/Option.hpp>
#include <vector>

namespace ledger {
	namespace core {
		namespace db {
			// Storage interfaces for blockchain
			class ReadOnlyBlockchainDB {
				virtual ~ReadOnlyBlockchainDB() = 0;
				// Get all blocks with height in [heightFrom, heightTo)
				virtual std::vector<DatabaseBlock> GetBlocks(int heightFrom, int heightTo) = 0;
				virtual Option<DatabaseBlockHeader> GetLastBlockHeader() = 0;
			};

			class BlockchainDB: public ReadOnlyBlockchainDB {
			public:
				virtual ~BlockchainDB() = 0;
				virtual void AddBlocks(const std::vector<DatabaseBlock>& blocks) = 0;
				// Remove all blocks with height in [heightFrom, heightTo)
				virtual void RemoveBlocks(int heightFrom, int heightTo) = 0;
				// Remove all blocks with height < heightTo
				virtual void RemoveBlocksUpTo(int heightTo) = 0;
			};
		}
	}
}

#endif //LEDGER_CORE_DB_BLOCKCHAINDB_HPP