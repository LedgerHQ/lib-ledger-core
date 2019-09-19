#include "UtilsCommandProcessor.h"
#include "messages/utils/commands.pb.h"
#include "async/Promise.hpp"
#include "math/CryptoUtils.h"
#include "math/Base58.hpp"
#include "crypto/SECP256k1Point.hpp"
#include "utils/hex.h"
#include "crypto/RIPEMD160.hpp"
#include "crypto/SHA256.hpp"

namespace ledger {
namespace core {
    using namespace message::utils;

    CreateXpubFromPointsResponse UtilsCommandProcessor::processRequest(const CreateXpubFromPointsRequest& req) {
        CreateXpubFromPointsResponse resp;
        SECP256k1Point point(hex::toByteArray(req.public_point()));
        std::vector<uint8_t> parentFingerprint{0,0,0,0};
        if (req.parent_public_point() != "") {
            SECP256k1Point parentPoint(hex::toByteArray(req.parent_public_point()));
            parentFingerprint = RIPEMD160::hash(SHA256::bytesToBytesHash(parentPoint.toByteArray(true)));
        }
        auto payload = SerializeExtendedKeyWithSHA256Checksum(
            hex::toByteArray(req.version_prefix()),
            req.depth(),
            parentFingerprint,
            req.index(),
            hex::toByteArray(req.chain_code()),
            point.toByteArray(true)
        );
        resp.set_xpub(Base58::encode(payload, Base58::BITCOIN_BASE58_DIGITS));
        return resp;
    }


    Future<std::string> UtilsCommandProcessor::processRequest(const std::string& request) {
        UtilsRequest req;
        if (!req.ParseFromString(request)) {
            throw std::runtime_error("Can't parse UtilsRequest");
        }
        switch (req.request_case())
        {
        case UtilsRequest::RequestCase::kXpubFromPoints: {
            return Future<std::string>::successful(processRequest(req.xpub_from_points()).SerializeAsString());
        }
        default:
            throw std::runtime_error("Unknown UtilsRequestType");
        }
    }
}
}