#ifndef LEDGER_CORE_DB_DATABASETRANSACTIONOUTPUT_HPP
#define LEDGER_CORE_DB_DATABASETRANSACTIONOUTPUT_HPP

#include <vector>
#include <string>

namespace ledger {
	namespace core {
		namespace db {

			struct TransactionOutputDescriptor {
				std::string hash;
				int32_t index;
			};

			// Representation of Transaction Output 
			struct TransactionOutput {
				TransactionOutputDescriptor desc;
				std::vector<int8_t> data;
			};

		}
	}
}

#endif //LEDGER_CORE_DB_DATABASETRANSACTIONOUTPUT_HPP