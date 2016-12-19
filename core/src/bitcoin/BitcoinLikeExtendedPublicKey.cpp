/*
 *
 * BitcoinLikeExtendedPublicKey
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
#include "BitcoinLikeExtendedPublicKey.hpp"
#include "../utils/djinni_helpers.hpp"
#include "../crypto/HASH160.hpp"

namespace ledger {
    namespace core {

        BitcoinLikeExtendedPublicKey::BitcoinLikeExtendedPublicKey(const api::BitcoinLikeNetworkParameters &params,
                                                                   const DeterministicPublicKey& key,
                                                                   const std::string& path) :
            CLONE_BITCOIN_LIKE_NETWORK_PARAMETERS(params, _params), _key(key), _path(path)
        {

        }

        std::shared_ptr<api::BitcoinLikeAddress> BitcoinLikeExtendedPublicKey::derive(const std::string &path) {
            return nullptr;
        }

        std::string BitcoinLikeExtendedPublicKey::toBase58() {
            return nullptr;
        }

        std::shared_ptr<api::BitcoinLikeExtendedPublicKey>
        BitcoinLikeExtendedPublicKey::fromPublicKeyCouple(const api::BitcoinLikeNetworkParameters &params,
                                                          const optional<std::vector<uint8_t>> &parentPublicKey,
                                                          const std::vector<uint8_t> &publicKey,
                                                          const std::string &path) {
            uint32_t parentFingerprint = 0;

            if (parentPublicKey) {
                auto hash160 = HASH160::hash(parentPublicKey.value());
                parentFingerprint = ((hash160[0] & 0xFFU) << 24) |
                                    ((hash160[1] & 0xFFU) << 16) |
                                    ((hash160[2] & 0xFFU) << 8) |
                                    (hash160[3] & 0xFFU);
            }
            BytesWriter writer;
            writer.writeByteArray(params.XPUBVersion);
            //writer.writeBeValue<uint32_t>();
            return std::shared_ptr<api::BitcoinLikeExtendedPublicKey>();
        }

        std::shared_ptr<api::BitcoinLikeExtendedPublicKey>
        api::BitcoinLikeExtendedPublicKey::fromBase58(const api::BitcoinLikeNetworkParameters &params,
                                                      const std::string &address) {

            return std::shared_ptr<api::BitcoinLikeExtendedPublicKey>();
        }
    }
}

/*
 *   derivePublicAddress(path, network) map {(result) =>
        val magic = network.getBip32HeaderPub
        val depth = path.length.toByte
        val childNum = path.childNum
        val chainCode = result.chainCode
        val publicKey = Crypto.compressPublicKey(result.publicKey)
        val rawXpub = new BytesWriter(13 + chainCode.length + publicKey.length)
        rawXpub.writeInt(magic)
        rawXpub.writeByte(depth)
        rawXpub.writeInt(fingerprint)
        rawXpub.writeInt(childNum)
        rawXpub.writeByteArray(chainCode)
        rawXpub.writeByteArray(publicKey)
        val xpub58 = Base58.encodeWitchChecksum(rawXpub.toByteArray)
        DeterministicKey.deserializeB58(xpub58, network)
      }
    }

    if (path.depth > 0) {
      derivePublicAddress(path.parent, network) flatMap {(result) =>
        val hash160 = Hash160.hash(Crypto.compressPublicKey(result.publicKey))
        val fingerprint: Long =
            ((hash160(0) & 0xFFL) << 24) |
            ((hash160(1) & 0xFFL) << 16) |
            ((hash160(2) & 0xFFL) << 8) |
            (hash160(3) & 0xFFL)
        finalize(fingerprint)
      }
    } else {
      finalize(0)
    }
 */