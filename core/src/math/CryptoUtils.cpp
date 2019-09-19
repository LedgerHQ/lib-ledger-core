#include "CryptoUtils.h"
#include "utils/Exception.hpp"
#include "crypto/SHA256.hpp"

namespace ledger {
    namespace core {


        std::vector<uint8_t> SerializeExtendedKeyWithSHA256Checksum(
            const std::vector<uint8_t>& version,
            uint8_t depth,
            const std::vector<uint8_t>& parentFingerPrint,
            uint32_t childNumber,
            const std::vector<uint8_t>& chainCode,
            const std::vector<uint8_t>& serializedKey) {
            std::vector<uint8_t> payload(4 + 1 + 4 + 4 + 32 + 33 + 4);
            if (version.size() < 4)
                throw Exception(api::ErrorCode::INVALID_VERSION, "Extended key serialization require version with 4 bytes");
            if (parentFingerPrint.size() < 4)
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Parent fingerprint should be 4 bytes long");
            if (chainCode.size() != 32)
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Chaincode should be 32 bytes long");
            if (serializedKey.size() != 33)
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Compressed key should be 33 bytes long");
            std::copy_n(version.begin(), 4, payload.begin());

            payload[4] = depth;
                        
            std::copy_n(parentFingerPrint.begin(), 4, payload.begin() + 5);
            
            payload[9] = (childNumber & 0xff000000) >> 24;
            payload[10] = (childNumber & 0x00ff0000) >> 16;
            payload[11] = (childNumber & 0x0000ff00) >> 8;
            payload[12] = (childNumber & 0x000000ff);
                        
            std::copy_n(chainCode.begin(), 32, payload.begin() + 13);

            std::copy_n(serializedKey.begin(), 33, payload.begin() + 45);
            auto checksum = SHA256::bytesToBytesHash(SHA256::dataToBytesHash(payload.data(), 78));
            std::copy_n(checksum.begin(), 4, payload.begin() + 78);
            return payload;
        }


    }
}