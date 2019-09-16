#include "CryptoUtils.h"
#include "../collections/vector.hpp"

namespace ledger {
    namespace core {
        std::string SerializeExtendedKey(
            const std::vector<uint8_t>& version,
            uint8_t depth,
            const std::vector<uint8_t>& parentFingerPrint,
            uint32_t childNumber,
            const std::vector<uint8_t>& chainCode,
            const std::vector<uint8_t>& serializedKey) {
            std::vector<>
        }
    }
}