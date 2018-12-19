#include <algorithm>

#include <database/UTXOSource.hpp>

namespace ledger {
    namespace core {
        UTXOSource::Key::Key(std::string htx, uint32_t i)
            : hashTX(htx), index(i) {
        }

        UTXOSource::Value::Value(BigInt satoshis, const std::string& address)
            : satoshis(satoshis), address(address) {
        }

        UTXOSource::SourceList::SourceList(std::map<Key, Value>&& available, std::vector<Key>&& spent)
            : available(available), spent(spent) {
        }

        void UTXOSource::invalidate() {
        }
    }
}
