#include <wallet/bitcoin/UTXO.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            UTXOValue::UTXOValue(const BigInt& amount, const std::string& address)
                : amount(amount), address(address) {
            }

            UTXOSourceList::UTXOSourceList(std::map<UTXOKey, UTXOValue>&& available, std::set<UTXOKey>&& spent, uint32_t height)
                : available(std::move(available)), spent(std::move(spent)), height(height) {
            }

            UTXOSourceList::UTXOSourceList(const std::map<UTXOKey, UTXOValue>& available, const std::set<UTXOKey>& spent, uint32_t height)
                : available(available), spent(spent), height(height) {
            }

        }
    }
}
