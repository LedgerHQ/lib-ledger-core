#ifndef LEDGER_CORE_DB_PARTIALBLOCKSDB_HPP
#define LEDGER_CORE_DB_PARTIALBLOCKSDB_HPP

#include <vector>
#include <database/DatabaseBlock.hpp>
#include <database/DatabaseTransaction.hpp>

namespace ledger {
	namespace core {
		namespace db {
			// Database interface for partial blocks
			class PartialBlocksDB {
			public:
				virtual ~PartialBlocksDB() = 0;
				virtual void Clean() = 0;
				virtual void AddTransactions(const DatabaseBlockHeader& block, const std::vector<DatabaseTransaction>& transactions) = 0;
				virtual std::vector<DatabaseTransaction> GetTransactions(const DatabaseBlockHeader& block) = 0;
				virtual void RemoveBlock(const DatabaseBlockHeader& block) = 0;
			};
		}
	}
}

#endif //LEDGER_CORE_DB_PARTIALBLOCKSDB_HPP