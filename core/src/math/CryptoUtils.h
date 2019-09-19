#include <string>
#include <vector>

namespace ledger {
    namespace core {

        // this is implementation of https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#serialization-format
        std::vector<uint8_t> SerializeExtendedKeyWithSHA256Checksum(
            const std::vector<uint8_t>& version,
            uint8_t depth,
            const std::vector<uint8_t>& parentFingerPrint,
            uint32_t childNumber,
            const std::vector<uint8_t>& chainCode,
            const std::vector<uint8_t>& serializedKey);
    }
}