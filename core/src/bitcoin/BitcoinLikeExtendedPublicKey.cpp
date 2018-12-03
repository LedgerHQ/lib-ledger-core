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
#include <crypto/SECP256k1Point.hpp>
namespace ledger {
    namespace core {

        BitcoinLikeExtendedPublicKey::BitcoinLikeExtendedPublicKey(const api::Currency &currency,
                                                                   const DeterministicPublicKey& key,
                                                                   const DerivationPath& path) :
            _currency(currency), _key(key), _path(path)
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
            auto key = _derive(0, p.toVector(), _key);
            return std::make_shared<BitcoinLikeAddress>(_currency, key.getPublicKeyHash160(), _currency.bitcoinLikeNetworkParameters.value().P2PKHVersion, optional<std::string>((_path + p).toString()));
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey> BitcoinLikeExtendedPublicKey::derive(
                const DerivationPath &path) {
            auto dpk = _derive(0, path.toVector(), _key);
            return std::make_shared<BitcoinLikeExtendedPublicKey>(_currency, dpk, _path + path);
        }

        std::string BitcoinLikeExtendedPublicKey::toBase58() {
            return Base58::encodeWithChecksum(_key.toByteArray(params().XPUBVersion), params().Identifier);
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey>
        BitcoinLikeExtendedPublicKey::fromRaw(const api::Currency &currency,
                                              const optional<std::vector<uint8_t>> &parentPublicKey,
                                              const std::vector<uint8_t> &publicKey,
                                              const std::vector<uint8_t> &chainCode,
                                              const std::string &path) {
            uint32_t parentFingerprint = 0;
            auto& params = currency.bitcoinLikeNetworkParameters.value();
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
            DeterministicPublicKey k(
                    pk.toByteArray(true), chainCode, p.getLastChildNum(), p.getDepth(), parentFingerprint, params.Identifier
            );
            return std::make_shared<BitcoinLikeExtendedPublicKey>(currency, k, p);
        }

        std::string BitcoinLikeExtendedPublicKey::getRootPath() {
            return _path.toString();
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey>
        BitcoinLikeExtendedPublicKey::fromBase58(const api::Currency &currency,
                                                 const std::string &xpubBase58, const Option<std::string> &path) {
            auto& params = currency.bitcoinLikeNetworkParameters.value();
            auto decodeResult = Base58::checkAndDecode(xpubBase58, params.Identifier);
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
                publicKey, chainCode, childNum, depth, fingerprint, params.Identifier
            );
            return std::make_shared<ledger::core::BitcoinLikeExtendedPublicKey>(currency, k, DerivationPath(path.getValueOr("m")));
        }

        std::vector<uint8_t> BitcoinLikeExtendedPublicKey::derivePublicKey(const std::string &path) {
            DerivationPath p(path);
            auto key = _derive(0, p.toVector(), _key);
            return key.getPublicKey();
        }

        std::vector<uint8_t> BitcoinLikeExtendedPublicKey::deriveHash160(const std::string &path) {
            DerivationPath p(path);
            auto key = _derive(0, p.toVector(), _key);
            return key.getPublicKeyHash160();
        }

        const api::BitcoinLikeNetworkParameters &BitcoinLikeExtendedPublicKey::params() const {
            return _currency.bitcoinLikeNetworkParameters.value();
        }

    }
}
