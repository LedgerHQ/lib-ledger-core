#include <algorithm>

#include <database/UTXOCache.hpp>

namespace ledger {
    namespace core {
        UTXOCache::Key::Key(std::string htx, uint32_t i)
            : hashTX(htx), index(i) {
        }

        UTXOCache::Value::Value(BigInt satoshis, const std::string& address)
            : satoshis(satoshis), address(address) {
        }

        UTXOCache::UTXOCache(uint32_t lowestHeight)
            : _lowestHeight(lowestHeight), _lastHeight(std::max(1u, lowestHeight) - 1) {
        }

        uint32_t UTXOCache::getLastHeight() {
            return _lastHeight;
        }

        uint32_t UTXOCache::getLowestHeight() {
            return _lowestHeight;
        }

        void UTXOCache::updateLastHeight(uint32_t lastHeight) {
            _lastHeight = std::max(_lastHeight, lastHeight);
        }
    }
}
