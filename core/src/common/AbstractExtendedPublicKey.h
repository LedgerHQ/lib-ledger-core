/*
 *
 * AbstractExtendedPublicKey
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#ifndef LEDGER_CORE_ABSTRACTEXTENDEDPUBLICKEY_H
#define LEDGER_CORE_ABSTRACTEXTENDEDPUBLICKEY_H

#include <memory>

#include <math/Base58.hpp>

#include <crypto/SECP256k1Point.hpp>
#include <crypto/HashAlgorithm.h>
#include <crypto/DeterministicPublicKey.hpp>
#include <crypto/RIPEMD160.hpp>

#include <utils/Option.hpp>
#include <utils/DerivationPath.hpp>

#include <bytes/BytesReader.h>

#include <api/Currency.hpp>
#include <collections/DynamicObject.hpp>
namespace ledger {
    namespace core {
        template <class NetworkParameters>
        class AbstractExtendedPublicKey {

        public:
            static inline DeterministicPublicKey _derive(int index, const std::vector<uint32_t>& childNums, const DeterministicPublicKey& key) {
                if (index >= childNums.size()) {
                    return key;
                }
                return _derive(index + 1, childNums, key.derive(childNums[index]));
            }

            std::string toBase58() {
                auto config = std::make_shared<DynamicObject>();
                config->putString("networkIdentifier", params().Identifier);
                return Base58::encodeWithChecksum(getKey().toByteArray(params().XPUBVersion), config);
            }

            static DeterministicPublicKey
            fromRaw(const api::Currency &currency,
                    const NetworkParameters &params,
                    const std::ledger_exp::optional<std::vector<uint8_t>> &parentPublicKey,
                    const std::vector<uint8_t> &publicKey,
                    const std::vector<uint8_t> &chainCode,
                    const std::string &path) {
                uint32_t parentFingerprint = 0;
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
                return DeterministicPublicKey(pk.toByteArray(true), chainCode, p.getLastChildNum(), p.getDepth(), parentFingerprint, params.Identifier);
            }

            static DeterministicPublicKey
            fromBase58(const api::Currency &currency,
                       const NetworkParameters &params,
                       const std::string &xpubBase58,
                       const Option<std::string> &path,
                       const std::string &networkBase58Dictionary = "") {
                //xpubBase58 should be composed of version(4) || depth(1) || fingerprint(4) || index(4) || chain(32) || key(33)
                auto config = std::make_shared<DynamicObject>();
                config->putString("networkIdentifier", params.Identifier);
                if (!networkBase58Dictionary.empty()) {
                    config->putString("base58Dictionary", networkBase58Dictionary);
                }
                auto decodeResult = Base58::checkAndDecode(xpubBase58, config);
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
                return DeterministicPublicKey(publicKey, chainCode, childNum, depth, fingerprint, params.Identifier);
            }

            std::vector<uint8_t> derivePublicKey(const std::string &path) {
                DerivationPath p(path);
                auto key = _derive(0, p.toVector(), getKey());
                return key.getPublicKey();
            }

            std::vector<uint8_t> deriveHash160(const std::string &path) {
                DerivationPath p(path);
                auto key = _derive(0, p.toVector(), getKey());
                return key.getPublicKeyHash160();
            }

        protected:
            virtual const NetworkParameters &params() const = 0;
            virtual const DeterministicPublicKey &getKey() const = 0;
            virtual const DerivationPath &getPath() const = 0;
            virtual const api::Currency &getCurrency() const = 0;
        };
    }
}


#endif //LEDGER_CORE_ABSTRACTEXTENDEDPUBLICKEY_H
