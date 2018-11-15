#ifndef LEDGER_CORE_DB_DATABASETRANSACTION_HPP
#define LEDGER_CORE_DB_DATABASETRANSACTION_HPP

#include <string>

namespace ledger {
	namespace core {
		namespace db {
			struct DatabaseTransaction {
				std::string hash;
				std::string data;
			};
		}
	}
}
#endif //LEDGER_CORE_DB_DATABASETRANSACTION_HPP