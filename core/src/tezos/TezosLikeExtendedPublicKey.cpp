/*
 *
 * TezosLikeExtendedPublicKey
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 25/04/2019.
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

#include "TezosLikeExtendedPublicKey.h"
#include "TezosLikeAddress.h"

#include <math/Base58.hpp>

#include <bytes/BytesReader.h>
#include <bytes/BytesWriter.h>

#include <utils/hex.h>
#include <utils/Exception.hpp>

#include <crypto/Keccak.h>
#include <crypto/SECP256k1Point.hpp>
#include <crypto/RIPEMD160.hpp>
#include <crypto/SHA256.hpp>
#include <crypto/HashAlgorithm.h>
#include <crypto/BLAKE.h>

namespace ledger {
    namespace core {

        TezosLikeExtendedPublicKey::TezosLikeExtendedPublicKey(const api::Currency &params,
                                                               const DeterministicPublicKey &key,
                                                               api::TezosCurve curve,
                                                               const DerivationPath &path) :
                _currency(params), _key(key), _path(path), _curve(curve) {}

        std::shared_ptr<api::TezosLikeAddress>
        TezosLikeExtendedPublicKey::derive(const std::string &path) {
            DerivationPath p(path);
            return std::make_shared<TezosLikeAddress>(_currency,
                                                      _key.getPublicKey(),
                                                      _currency.tezosLikeNetworkParameters.value().ImplicitPrefix,
                                                      _curve,
                                                      optional<std::string>((_path + p).toString()));
        }

        std::shared_ptr<TezosLikeExtendedPublicKey>
        TezosLikeExtendedPublicKey::derive(const DerivationPath &path) {
            return std::make_shared<TezosLikeExtendedPublicKey>(_currency, _key, _curve, _path + path);
        }

        std::vector<uint8_t>
        TezosLikeExtendedPublicKey::derivePublicKey(const std::string &path) {
            return _key.getPublicKey();
        }

        std::vector<uint8_t>
        TezosLikeExtendedPublicKey::deriveHash160(const std::string &path) {
            return _key.getPublicKeyBlake2b(_curve == api::TezosCurve::ED25519);
        }

        std::string TezosLikeExtendedPublicKey::toBase58()
        {
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", "xtz");
            // FIXME: The key is 33bytes long even for ed25519, need to manually remove the leading byte
            std::vector<uint8_t> key = _key.getPublicKey();
            key.erase(key.begin());
            // FIXME: Hardcoded to the edpk (ed25519 public key) prefix. It should be stored elsewhere
            std::vector<uint8_t> payload = vector::concat({0x0D, 0x0F, 0x25, 0xD9}, key);
            auto b58encoded = Base58::encodeWithChecksum(payload, config);
            return b58encoded;
        }

        std::string TezosLikeExtendedPublicKey::getRootPath() {
            return _path.toString();
        }

        std::shared_ptr<TezosLikeExtendedPublicKey>
        TezosLikeExtendedPublicKey::fromRaw(const api::Currency &currency,
                                            const optional<std::vector<uint8_t>> &parentPublicKey,
                                            const std::vector<uint8_t> &publicKey,
                                            const std::vector<uint8_t> &chainCode,
                                            const std::string &path,
                                            api::TezosCurve curve)
        {
            auto &params = currency.tezosLikeNetworkParameters.value();
            DeterministicPublicKey k = DeterministicPublicKey(publicKey, chainCode, 0, 0, 0, params.Identifier);
            return std::make_shared<TezosLikeExtendedPublicKey>(currency, k, curve, DerivationPath(path));
        }

        std::shared_ptr<TezosLikeExtendedPublicKey>
        TezosLikeExtendedPublicKey::fromBase58(const api::Currency &currency,
                                               const std::string &xpubBase58,
                                               const Option<std::string> &path) {
            auto &params = currency.tezosLikeNetworkParameters.value();
            if (xpubBase58.find("edpk") != std::string::npos) {
                auto config = api::DynamicObject::newInstance();
                auto decodeResult = Base58::checkAndDecode(xpubBase58, config);

                BytesReader reader(decodeResult.getValue());
                auto version = reader.read(3);
                auto publicKey = reader.readUntilEnd();

                DeterministicPublicKey k(publicKey, {}, 0, 0, 0, params.Identifier);
                return std::make_shared<ledger::core::TezosLikeExtendedPublicKey>(currency, k, api::TezosCurve::ED25519, DerivationPath(path.getValueOr("m")));
            }
            DeterministicPublicKey k = TezosExtendedPublicKey::fromBase58(currency, params, xpubBase58, path);
            return std::make_shared<ledger::core::TezosLikeExtendedPublicKey>(currency, k, k.getPublicKey().size() == 33 ? api::TezosCurve::ED25519 : api::TezosCurve::SECP256K1, DerivationPath(path.getValueOr("m")));
        }

    }
}
