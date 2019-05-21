/*
 *
 * RippleLikeExtendedPublicKey
 *
 * Created by El Khalil Bellakrid on 05/01/2019.
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

#include <core/math/Base58.hpp>
#include <core/bytes/BytesReader.h>
#include <core/bytes/BytesWriter.h>
#include <core/collections/vector.hpp>
#include <core/crypto/SHA256.hpp>

#include <RippleNetworks.h>
#include <RippleLikeExtendedPublicKey.h>

namespace ledger {
    namespace core {
        //TODO: support addresses resulting from Ed25519
        RippleLikeExtendedPublicKey::RippleLikeExtendedPublicKey(const api::Currency& params,
                                                                 const DeterministicPublicKey& key,
                                                                 const DerivationPath& path):
                _currency(params), _key(key), _path(path)
        {}
        std::shared_ptr<api::RippleLikeAddress> RippleLikeExtendedPublicKey::derive(const std::string & path) {
            DerivationPath p(path);
            auto key = _derive(0, p.toVector(), _key);
            return std::make_shared<RippleLikeAddress>(_currency, key.getPublicKeyHash160(), std::vector<uint8_t>({0x00}), optional<std::string>((_path + p).toString()));
        }

        std::shared_ptr<RippleLikeExtendedPublicKey> RippleLikeExtendedPublicKey::derive(const DerivationPath &path) {
            auto dpk = _derive(0, path.toVector(), _key);
            return std::make_shared<RippleLikeExtendedPublicKey>(_currency, dpk, _path + path);
        }

        std::vector<uint8_t> RippleLikeExtendedPublicKey::derivePublicKey(const std::string & path) {
            return RippleExtendedPublicKey::derivePublicKey(path);
        }

        std::vector<uint8_t> RippleLikeExtendedPublicKey::deriveHash160(const std::string & path) {
            return RippleExtendedPublicKey::deriveHash160(path);
        }

        std::string RippleLikeExtendedPublicKey::toBase58() {
            return RippleExtendedPublicKey::toBase58();
        }

        std::string RippleLikeExtendedPublicKey::getRootPath() {
            return _path.toString();
        }

        std::shared_ptr<RippleLikeExtendedPublicKey>
        RippleLikeExtendedPublicKey::fromRaw(const api::Currency& currency,
                                               const optional<std::vector<uint8_t>>& parentPublicKey,
                                               const std::vector<uint8_t>& publicKey,
                                               const std::vector<uint8_t> &chainCode,
                                               const std::string& path) {
            auto& params = currency.rippleLikeNetworkParameters.value();
            DeterministicPublicKey k = RippleExtendedPublicKey::fromRaw(currency, params, parentPublicKey, publicKey, chainCode, path);
            DerivationPath p(path);
            return std::make_shared<RippleLikeExtendedPublicKey>(currency, k, p);
        }

        std::shared_ptr<RippleLikeExtendedPublicKey>
        RippleLikeExtendedPublicKey::fromBase58(const api::Currency& currency,
                                                  const std::string& xpubBase58,
                                                  const Option<std::string>& path) {
            auto& params = currency.rippleLikeNetworkParameters.value();
            DeterministicPublicKey k = RippleExtendedPublicKey::fromBase58(currency, params, xpubBase58, path, networks::RIPPLE_DIGITS);
            return std::make_shared<ledger::core::RippleLikeExtendedPublicKey>(currency, k, DerivationPath(path.getValueOr("m")));

        }

    }
}
