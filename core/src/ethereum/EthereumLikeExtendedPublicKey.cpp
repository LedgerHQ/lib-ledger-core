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
#include <math/Base58.hpp>
#include <bytes/BytesReader.h>
#include <bytes/BytesWriter.h>
#include <utils/Exception.hpp>
#include "EthereumLikeAddress.h"
#include <crypto/Keccak.h>

namespace ledger {
    namespace core {

        EthereumLikeExtendedPublicKey::EthereumLikeExtendedPublicKey(const api::Currency& params,
                                                                     const DeterministicPublicKey& key,
                                                                     const DerivationPath& path):
            _currency(params), _key(key), _path(path)
        {

        }

        static inline DeterministicPublicKey _derive(int index, const std::vector<uint32_t>& childNums, const DeterministicPublicKey& key) {
            if (index >= childNums.size()) {
                return key;
            }
            return _derive(index + 1, childNums, key.derive(childNums[index]));
        }

        std::shared_ptr<api::EthereumLikeAddress> EthereumLikeExtendedPublicKey::derive(const std::string & path) {
            DerivationPath p(path);
            auto key = _derive(0, p.toVector(), _key);
            return std::make_shared<EthereumLikeAddress>(_currency, key.getPublicKeyKeccak256(), optional<std::string>((_path + p).toString()));
        }

        std::shared_ptr<EthereumLikeExtendedPublicKey> EthereumLikeExtendedPublicKey::derive(const DerivationPath &path) {
            auto dpk = _derive(0, path.toVector(), _key);
            return std::make_shared<EthereumLikeExtendedPublicKey>(_currency, dpk, _path + path);
        }

        std::vector<uint8_t> EthereumLikeExtendedPublicKey::derivePublicKey(const std::string & path) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeExtendedPublicKey::derivePublicKey is not implemented yet");
        }

        std::vector<uint8_t> EthereumLikeExtendedPublicKey::deriveHash160(const std::string & path) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeExtendedPublicKey::deriveHash160 is not implemented yet");
        }

        std::string EthereumLikeExtendedPublicKey::toBase58() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeExtendedPublicKey::toBase58 is not implemented yet");
        }

        std::string EthereumLikeExtendedPublicKey::getRootPath() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeExtendedPublicKey::getRootPath is not implemented yet");
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

            DeterministicPublicKey k(publicKey, chainCode, childNum, depth, fingerprint);
            return std::make_shared<ledger::core::EthereumLikeExtendedPublicKey>(currency, k, DerivationPath(path.getValueOr("m")));

        }
    }
}