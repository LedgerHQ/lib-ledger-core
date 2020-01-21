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


#include <core/api/Configuration.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/bytes/BytesReader.hpp>
#include <core/bytes/BytesWriter.hpp>
#include <core/crypto/HASH160.hpp>
#include <core/crypto/SECP256k1Point.hpp>
#include <core/debug/Benchmarker.hpp>
#include <core/math/Base58.hpp>
#include <core/utils/DerivationPath.hpp>
#include <core/utils/DjinniHelpers.hpp>

#include <bitcoin/BitcoinLikeAddress.hpp>
#include <bitcoin/BitcoinLikeExtendedPublicKey.hpp>

namespace ledger {
    namespace core {

        BitcoinLikeExtendedPublicKey::BitcoinLikeExtendedPublicKey(const api::Currency &currency,
                                                                   const DeterministicPublicKey& key,
                                                                   const std::shared_ptr<DynamicObject> &configuration,
                                                                   const DerivationPath& path) :
            _currency(currency), _key(key), _configuration(configuration), _path(path)
        {}

        std::shared_ptr<api::BitcoinLikeAddress> BitcoinLikeExtendedPublicKey::derive(const std::string &path) {
            DerivationPath p(path);
            auto key = _derive(0, p.toVector(), _key);
            auto keychainEngine = _configuration->getString(api::Configuration::KEYCHAIN_ENGINE).value_or(api::KeychainEngines::BIP32_P2PKH);
            return std::make_shared<BitcoinLikeAddress>(_currency,
                                                        key.getPublicKeyHash160(),
                                                        keychainEngine,
                                                        optional<std::string>((_path + p).toString()));
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey> BitcoinLikeExtendedPublicKey::derive(const DerivationPath &path) {
            auto dpk = _derive(0, path.toVector(), _key);
            return std::make_shared<BitcoinLikeExtendedPublicKey>(_currency, dpk, _configuration, _path + path);
        }

        std::string BitcoinLikeExtendedPublicKey::toBase58() {
            return BitcoinExtendedPublicKey::toBase58();
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey>
        BitcoinLikeExtendedPublicKey::fromRaw(const api::Currency &currency,
                                              const optional<std::vector<uint8_t>> &parentPublicKey,
                                              const std::vector<uint8_t> &publicKey,
                                              const std::vector<uint8_t> &chainCode,
                                              const std::string &path,
                                              const std::shared_ptr<DynamicObject> &configuration) {
            auto& params = networks::getBitcoinLikeNetworkParameters(currency.name);
            DeterministicPublicKey k = BitcoinExtendedPublicKey::fromRaw(currency, params, parentPublicKey, publicKey, chainCode, path);
            DerivationPath p(path);
            return std::make_shared<BitcoinLikeExtendedPublicKey>(currency, k, configuration, p);
        }

        std::string BitcoinLikeExtendedPublicKey::getRootPath() {
            return _path.toString();
        }

        std::shared_ptr<BitcoinLikeExtendedPublicKey>
        BitcoinLikeExtendedPublicKey::fromBase58(const api::Currency &currency,
                                                 const std::string &xpubBase58,
                                                 const Option<std::string> &path,
                                                 const std::shared_ptr<DynamicObject> &configuration) {
            auto& params = networks::getBitcoinLikeNetworkParameters(currency.name);
            DeterministicPublicKey k = BitcoinExtendedPublicKey::fromBase58(currency, params, xpubBase58, path);
            return std::make_shared<ledger::core::BitcoinLikeExtendedPublicKey>(currency, k, configuration, DerivationPath(path.getValueOr("m")));
        }

        std::vector<uint8_t> BitcoinLikeExtendedPublicKey::derivePublicKey(const std::string &path) {
            return BitcoinExtendedPublicKey::derivePublicKey(path);
        }

        std::vector<uint8_t> BitcoinLikeExtendedPublicKey::deriveHash160(const std::string &path) {
            return BitcoinExtendedPublicKey::deriveHash160(path);
        }

    }
}
