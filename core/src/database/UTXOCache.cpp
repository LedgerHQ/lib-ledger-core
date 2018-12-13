#include <database/UTXOCache.hpp>

namespace ledger {
    namespace core {
        UTXOCache::UTXOCache(uint32_t lowestHeight)
            : _lowestHeight(lowestHeight), _lastHeight(std::max(1, lowestHeight) - 1) {
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
