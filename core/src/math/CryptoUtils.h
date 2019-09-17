#include <string>
#include <vector>

namespace ledger {
    namespace core {
        // this is implementation of https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#serialization-format
        template<typename Base58Encoder>
        std::string SerializeExtendedKey(
            const std::vector<uint8_t>& version,
            uint8_t depth,
            const std::vector<uint8_t>& parentFingerPrint,
            uint32_t childNumber,
            const std::vector<uint8_t>& chainCode,
            const std::vector<uint8_t>& serializedKey) {
            std::vector<uint8_t> payload(4 + 1 + 4 + 4 + 32 + 33);
            std::copy(version.begin(), version.end(), payload.begin());
            payload[4] = depth;
            std::copy(parentFingerPrint.begin(), parentFingerPrint.end(), payload.begin() + 5);

        }
    }
}