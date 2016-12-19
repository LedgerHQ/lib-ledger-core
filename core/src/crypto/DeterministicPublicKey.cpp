/*
 *
 * DeterministicPublicKey
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/12/2016.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "../bytes/BytesWriter.h"
#include "DeterministicPublicKey.hpp"
#include "RIPEMD160.hpp"
#include "../utils/Exception.hpp"
#include "HMAC.hpp"
#include "../math/BigInt.h"
#include "SECP256k1Point.hpp"

namespace ledger {
    namespace core {

        static auto N = BigInt::fromHex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");

        DeterministicPublicKey::DeterministicPublicKey(const std::vector<uint8_t> &publicKey,
                                                       const std::vector<uint8_t> &chainCode, uint32_t childNum,
                                                       uint32_t depth, uint32_t parentFingerprint) :
            _key(publicKey), _chainCode(chainCode), _childNum(childNum), _depth(depth), _parentFingerprint(parentFingerprint)
        {

        }



        uint32_t DeterministicPublicKey::getFingerprint() const {
            auto hash160 = getPublicKeyHash160();
            return (uint32_t)hash160[0] << 24 |
                   (uint32_t)hash160[1] << 16 |
                   (uint32_t)hash160[2] << 8  |
                    (uint32_t)hash160[3];
        }

        std::vector<uint8_t> DeterministicPublicKey::getUncompressedPublicKey() const {
            return std::vector<uint8_t>();
        }

        std::vector<uint8_t> DeterministicPublicKey::getPublicKeyHash160() const {
            return RIPEMD160::hash(_key);
        }

        const std::vector<uint8_t>& DeterministicPublicKey::getPublicKey() const {
            return _key;
        }

        DeterministicPublicKey DeterministicPublicKey::derive(uint32_t childIndex) const {
            if (childIndex & 0x80000000) {
                throw Exception(api::ErrorCode::PRIVATE_DERIVATION_NOT_SUPPORTED, "Private derivation is not supported by DeterministicPublicKey");
            }
            BytesWriter data;
            data.writeByteArray(_key);
            data.writeBeValue<uint32_t>(childIndex);

            auto I = HMAC::sha512(_chainCode, data.toByteArray());
            BigInt IL(std::vector<uint8_t >(I.begin(), I.begin() + 32), false);
            std::vector<uint8_t> IR(I.begin() + 32, I.end());

            if (IL >= N) {
                throw Exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Cannot derive key - IL >= N");
            }

            SECP256k1Point K = SECP256k1Point(_key).generatorMultiply(IL.toByteArray());

            return DeterministicPublicKey(
                    K.toByteArray(),
                    IR,
                    childIndex,
                    _depth + 1,
                    getFingerprint()
            );
        }
    }
}