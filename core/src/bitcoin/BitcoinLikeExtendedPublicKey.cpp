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
#include <src/utils/DerivationPath.hpp>
#include <debug/Benchmarker.h>
#include "../bytes/BytesWriter.h"
#include "BitcoinLikeExtendedPublicKey.hpp"
#include "../utils/djinni_helpers.hpp"
#include "../crypto/HASH160.hpp"
#include "../math/Base58.hpp"
#include "../bytes/BytesReader.h"
#include "BitcoinLikeAddress.hpp"

namespace ledger {
    namespace core {

        BitcoinLikeExtendedPublicKey::BitcoinLikeExtendedPublicKey(const api::BitcoinLikeNetworkParameters &params,
                                                                   const DeterministicPublicKey& key,
                                                                   const DerivationPath& path) :
            _params(params), _key(key), _path(path)
        {

        }

        static inline DeterministicPublicKey _derive(int index, const std::vector<uint32_t>& childNums, const DeterministicPublicKey& key) {
            if (index >= childNums.size()) {
                return key;
            }
            return _derive(index + 1, childNums, key.derive(childNums[index]));
        }

        std::shared_ptr<api::BitcoinLikeAddress> BitcoinLikeExtendedPublicKey::derive(const std::string &path) {
            DerivationPath p(path);
            Benchmarker benchmarker("Derivation", nullptr);
            benchmarker.start();
            auto key = _derive(0, p.toVector(), _key);
            benchmarker.stop();
            return std::make_shared<BitcoinLikeAddress>(_params, key.getPublicKeyHash160(), _params.P2PKHVersion, optional<std::string>((_path + p).toString()));
        }

        std::string BitcoinLikeExtendedPublicKey::toBase58() {
            return Base58::encodeWithChecksum(_key.toByteArray(_params.XPUBVersion));
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey>
        BitcoinLikeExtendedPublicKey::fromRaw(const api::BitcoinLikeNetworkParameters &params,
                                              const optional<std::vector<uint8_t>> &parentPublicKey,
                                              const std::vector<uint8_t> &publicKey,
                                              const std::vector<uint8_t> &chainCode,
                                              const std::string &path) {
            uint32_t parentFingerprint = 0;

            if (parentPublicKey) {
                auto hash160 = HASH160::hash(parentPublicKey.value());
                parentFingerprint = ((hash160[0] & 0xFFU) << 24) |
                                    ((hash160[1] & 0xFFU) << 16) |
                                    ((hash160[2] & 0xFFU) << 8) |
                                    (hash160[3] & 0xFFU);
            }
            DerivationPath p(path);

            DeterministicPublicKey k(
                    publicKey, chainCode, p.getLastChildNum(), p.getDepth(), parentFingerprint
            );
            return std::make_shared<BitcoinLikeExtendedPublicKey>(params, k, p);
        }

        std::string BitcoinLikeExtendedPublicKey::getRootPath() {
            return _path.toString();
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey>
        BitcoinLikeExtendedPublicKey::fromBase58(const api::BitcoinLikeNetworkParameters &params,
                                                 const std::string &xpubBase58, const Option<std::string> &path) {
            auto decodeResult = Base58::checkAndDecode(xpubBase58);
            if (decodeResult.isFailure())
                throw decodeResult.getFailure();
            BytesReader reader(decodeResult.getValue());
            auto version = reader.read(params.XPUBVersion.size());
            if (version != params.XPUBVersion) {
                throw  Exception(api::ErrorCode::INVALID_NETWORK_ADDRESS_VERSION, "Provided network parameters and address version do not match.");
            }
            auto depth = reader.readNextByte();
            auto fingerprint = reader.readNextBeUint();
            auto childNum = reader.readNextBeUint();
            auto chainCode = reader.read(32);
            auto publicKey = reader.readUntilEnd();
            DeterministicPublicKey k(
                publicKey, chainCode, childNum, depth, fingerprint
            );
            return std::make_shared<ledger::core::BitcoinLikeExtendedPublicKey>(params, k, path.getValueOr("m"));
        }

        std::shared_ptr<api::BitcoinLikeExtendedPublicKey>
        api::BitcoinLikeExtendedPublicKey::fromBase58(const api::BitcoinLikeNetworkParameters &params,
                                                      const std::string &address,
                                                      const std::experimental::optional<std::string> & path) {
            return ledger::core::BitcoinLikeExtendedPublicKey::fromBase58(params, address, path);
        }
    }
}
