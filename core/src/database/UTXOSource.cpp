#include <algorithm>

#include <database/UTXOSource.hpp>

namespace ledger {
    namespace core {
        UTXOSource::Value::Value(BigInt amount, const std::string& address)
            : amount(amount), address(address) {
        }

        UTXOSource::SourceList::SourceList(std::map<Key, Value>&& available, std::vector<Key>&& spent)
            : available(available), spent(spent) {
        }

        void UTXOSource::invalidate() {
        }
    }
}
