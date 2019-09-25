//
// Created by PIERRE POLLASTRI on 25/08/2017.
//

#include <core/crypto/Secp256k1Api.hpp>
#include <core/utils/Exception.hpp>
#include <core/utils/Hex.hpp>
#include <include/secp256k1.h>

namespace ledger {
    namespace core {
        std::shared_ptr<api::Secp256k1> ledger::core::api::Secp256k1::createInstance() {
            return std::make_shared<Secp256k1Api>();
        }

        Secp256k1Api::Secp256k1Api() {
            _context = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN);
        }

        std::vector<uint8_t> Secp256k1Api::computePubKey(const std::vector<uint8_t> &privKey, bool compress) {
            secp256k1_pubkey pk;
            size_t outLength = 65;
            std::vector<uint8_t> out(outLength);

            if (secp256k1_ec_pubkey_create(_context, &pk, (const unsigned char *)privKey.data()) != 1) {
                throw make_exception(api::ErrorCode::EC_PRIV_KEY_INVALID_FORMAT, "EC private key {} is not valid",
                hex::toString(privKey));
            }
            secp256k1_ec_pubkey_serialize(_context, (unsigned char *) out.data(), &outLength, &pk,
                                          compress ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED);
            out.resize(outLength);
            return out;
        }

        std::vector<uint8_t> Secp256k1Api::computeUncompressedPubKey(const std::vector<uint8_t> & pubKey) {
            secp256k1_pubkey pk;
            if (secp256k1_ec_pubkey_parse(_context, &pk, pubKey.data(), pubKey.size()) == -1) {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Unable to parse secp256k1 point");
            }
            size_t outLength = 65;
            std::vector<uint8_t> out(outLength);
            secp256k1_ec_pubkey_serialize(_context, (unsigned char *) out.data(), &outLength, &pk, SECP256K1_EC_UNCOMPRESSED);
            out.resize(outLength);
            return out;
        }

        std::vector<uint8_t> Secp256k1Api::sign(const std::vector<uint8_t> &privKey, const std::vector<uint8_t> &data) {
            if (privKey.size() != 32 || data.size() != 32)
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Data and private key must have a length of 32 bytes");
            secp256k1_ecdsa_signature sig;

            if (secp256k1_ecdsa_sign(_context, &sig, (unsigned char *)data.data(), (unsigned char *)privKey.data(), NULL, NULL) != 1) {
                throw make_exception(api::ErrorCode::EC_SIGN_FAILED, "Fatal error during message signature.");
            }
            size_t outLength = 72;
            std::vector<uint8_t> out(outLength);
            secp256k1_ecdsa_signature_serialize_der(_context, (unsigned char *)out.data(), &outLength, &sig);
            out.resize(outLength);
            return out;
        }

        bool Secp256k1Api::verify(const std::vector<uint8_t> &data, const std::vector<uint8_t>& signature, const std::vector<uint8_t> &pubKey) {
            if (data.size() != 32)
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Data must have a length of 32 bytes");
            secp256k1_pubkey pk;
            secp256k1_ecdsa_signature sig;


            if (secp256k1_ec_pubkey_parse(_context, &pk, pubKey.data(), pubKey.size()) != 1) {
                throw make_exception(api::ErrorCode::EC_PUB_KEY_INVALID, "EC public key {} is not valid.", hex::toString(pubKey));
            }
            if (secp256k1_ecdsa_signature_parse_der(_context, &sig, signature.data(), signature.size()) != 1) {
                throw make_exception(api::ErrorCode::EC_DER_SIGNATURE_INVALID, "EC DER {} is not valid.", hex::toString(signature));
            }
            return secp256k1_ecdsa_verify(_context, &sig, data.data(), &pk) == 1;
        }

        Secp256k1Api::~Secp256k1Api() {
             secp256k1_context_destroy(_context);
        }

        std::shared_ptr<api::Secp256k1> api::Secp256k1::newInstance() {
            return std::make_shared<Secp256k1Api>();
        }
    }
}
