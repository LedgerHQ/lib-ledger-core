#ifndef LEDGER_CORE_DB_UTXODATABASE_HPP
#define LEDGER_CORE_DB_UTXODATABASE_HPP

#include <database/DatabaseTransactionOutput.hpp>
#include <vector>

namespace ledger {
	namespace core {
		namespace db {

			class ReadOnlyUTXODatabase {
				virtual ~ReadOnlyUTXODatabase() = 0;
				virtual std::vector<TransactionOutput> GetUTXOs() = 0;
			};

			class UTXODatabase : public ReadOnlyUTXODatabase {
			public:
				virtual ~UTXODatabase() = 0;
				virtual void AddTransactions(const std::vector<TransactionOutputDescriptor>& inputs, const std::vector<TransactionOutput>& outputs) = 0;
			};
		}
	}
}

#endif //LEDGER_CORE_DB_UTXODATABASE_HPP