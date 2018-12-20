#include <wallet/bitcoin/UTXO.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            UTXOValue::UTXOValue(BigInt amount, const std::string& address)
                : amount(amount), address(address) {
            }

            UTXOSourceList::UTXOSourceList(std::map<UTXOKey, UTXOValue>&& available, std::set<UTXOKey>&& spent)
                : available(available), spent(spent) {
            }
        }
    }
}
