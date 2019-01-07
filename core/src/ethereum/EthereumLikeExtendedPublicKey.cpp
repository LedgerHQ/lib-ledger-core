/*
 *
 * EthereumLikeExtendedPublicKey
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 08/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "EthereumLikeExtendedPublicKey.h"
#include "EthereumLikeAddress.h"

#include <math/Base58.hpp>

#include <bytes/BytesReader.h>
#include <bytes/BytesWriter.h>

#include <utils/Exception.hpp>

#include <crypto/Keccak.h>
#include <crypto/SECP256k1Point.hpp>
#include <crypto/RIPEMD160.hpp>
#include <crypto/SHA256.hpp>
#include <crypto/HashAlgorithm.h>

namespace ledger {
    namespace core {

        EthereumLikeExtendedPublicKey::EthereumLikeExtendedPublicKey(const api::Currency& params,
                                                                     const DeterministicPublicKey& key,
                                                                     const DerivationPath& path):
            _currency(params), _key(key), _path(path)
        {}

        std::shared_ptr<api::EthereumLikeAddress> EthereumLikeExtendedPublicKey::derive(const std::string & path) {
            DerivationPath p(path);
            return std::make_shared<EthereumLikeAddress>(_currency, _key.getPublicKeyKeccak256(), optional<std::string>((_path + p).toString()));
        }

        std::shared_ptr<EthereumLikeExtendedPublicKey> EthereumLikeExtendedPublicKey::derive(const DerivationPath &path) {
            return std::make_shared<EthereumLikeExtendedPublicKey>(_currency, _key, _path + path);
        }

        std::vector<uint8_t> EthereumLikeExtendedPublicKey::derivePublicKey(const std::string & path) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeExtendedPublicKey::derivePublicKey is not implemented yet");
        }

        std::vector<uint8_t> EthereumLikeExtendedPublicKey::deriveHash160(const std::string & path) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeExtendedPublicKey::deriveHash160 is not implemented yet");
        }

        std::string EthereumLikeExtendedPublicKey::toBase58() {
            return Base58::encodeWithChecksum(_key.toByteArray(_currency.ethereumLikeNetworkParameters.value().XPUBVersion),_currency.ethereumLikeNetworkParameters.value().Identifier);
        }

        std::string EthereumLikeExtendedPublicKey::getRootPath() {
            return _path.toString();
        }

        std::shared_ptr<EthereumLikeExtendedPublicKey>
        EthereumLikeExtendedPublicKey::fromRaw(const api::Currency& currency,
                const optional<std::vector<uint8_t>>& parentPublicKey,
                const std::vector<uint8_t>& publicKey,
                const std::vector<uint8_t> &chainCode,
                const std::string& path) {
            uint32_t parentFingerprint = 0;
            auto& params = currency.ethereumLikeNetworkParameters.value();
            SECP256k1Point pk(publicKey);
            if (parentPublicKey) {
                SECP256k1Point ppp(parentPublicKey.value());
                HashAlgorithm hashAlgorithm(params.Identifier);
                auto hash = hashAlgorithm.bytesToBytesHash(ppp.toByteArray(true));
                hash = RIPEMD160::hash(hash);
                parentFingerprint = ((hash[0] & 0xFFU) << 24) |
                                    ((hash[1] & 0xFFU) << 16) |
                                    ((hash[2] & 0xFFU) << 8) |
                                    (hash[3] & 0xFFU);
            }
            DerivationPath p(path);
            DeterministicPublicKey k(pk.toByteArray(true), chainCode, p.getLastChildNum(), p.getDepth(), parentFingerprint, params.Identifier);
            return std::make_shared<EthereumLikeExtendedPublicKey>(currency, k, p);
        }

        std::shared_ptr<EthereumLikeExtendedPublicKey>
        EthereumLikeExtendedPublicKey::fromBase58(const api::Currency& currency,
                                                  const std::string& xpubBase58,
                                                  const Option<std::string>& path) {
            //xpubBase58 should be composed of version(4) || depth(1) || fingerprint(4) || index(4) || chain(32) || key(33)
            auto& params = currency.ethereumLikeNetworkParameters.value();
            auto decodeResult = Base58::checkAndDecode(xpubBase58);
            if (decodeResult.isFailure())
                    throw decodeResult.getFailure();
            BytesReader reader(decodeResult.getValue());

            //4 bytes of version
            auto version = reader.read(params.XPUBVersion.size());

            if (version != params.XPUBVersion) {
                    throw  Exception(api::ErrorCode::INVALID_NETWORK_ADDRESS_VERSION, "Provided network parameters and address version do not match.");
            }
            //1 byte of depth
            auto depth = reader.readNextByte();
            //4 bytes of fingerprint
            auto fingerprint = reader.readNextBeUint();
            //4 bytes of child's index
            auto childNum = reader.readNextBeUint();
            //32 bytes of chaincode
            auto chainCode = reader.read(32);
            //33 bytes of publicKey
            auto publicKey = reader.readUntilEnd();

            DeterministicPublicKey k(publicKey, chainCode, childNum, depth, fingerprint, currency.ethereumLikeNetworkParameters.value().Identifier);
            return std::make_shared<ledger::core::EthereumLikeExtendedPublicKey>(currency, k, DerivationPath(path.getValueOr("m")));

        }

    }
}