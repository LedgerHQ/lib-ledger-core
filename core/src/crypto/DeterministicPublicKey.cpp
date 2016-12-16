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
#include "DeterministicPublicKey.hpp"
#include "RIPEMD160.hpp"
#include "../utils/Exception.hpp"

namespace ledger {
    namespace core {

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

            }

            /**
             *  if (!valid_) throw InvalidHDKeychainException();

    bool priv_derivation = 0x80000000 & i;
    if (!isPrivate() && priv_derivation) {
        throw std::runtime_error("Cannot do private key derivation on public key.");
    }

    HDKeychain child;
    child.valid_ = false;

    uchar_vector data;
    data += priv_derivation ? key_ : pubkey_;
    data.push_back(i >> 24);
    data.push_back((i >> 16) & 0xff);
    data.push_back((i >> 8) & 0xff);
    data.push_back(i & 0xff);

    bytes_t digest = hmac_sha512(chain_code_, data);
    bytes_t left32(digest.begin(), digest.begin() + 32);
    BigInt Il(left32);
    if (Il >= CURVE_ORDER) throw InvalidHDKeychainException();

    // The following line is used to test behavior for invalid indices
    // if (rand() % 100 < 10) return child;

    if (isPrivate()) {
        BigInt k(key_);
        k += Il;
        k %= CURVE_ORDER;
        if (k.isZero()) throw InvalidHDKeychainException();

        bytes_t child_key = k.getBytes();
        // pad with 0's to make it 33 bytes
        uchar_vector padded_key(33 - child_key.size(), 0);
        padded_key += child_key;
        child.key_ = padded_key;
        child.updatePubkey();
    }
    else {
        secp256k1_point K;
        K.bytes(pubkey_);
        K.generator_mul(left32);
        if (K.is_at_infinity()) throw InvalidHDKeychainException();

        child.key_ = child.pubkey_ = K.bytes();
    }

    child.version_ = version_;
    child.depth_ = depth_ + 1;
    child.parent_fp_ = fp();
    child.child_num_ = i;
    child.chain_code_.assign(digest.begin() + 32, digest.end());

    child.valid_ = true;
    return child;
             */
            return *this;
        }
    }
}