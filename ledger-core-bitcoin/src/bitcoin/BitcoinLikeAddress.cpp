/*
 *
 * BitcoinLikeAddress
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
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
#include <core/bytes/BytesWriter.hpp>
#include <core/collections/DynamicObject.hpp>
#include <core/collections/Vector.hpp>
#include <core/crypto/HashAlgorithm.hpp>
#include <core/crypto/HASH160.hpp>
#include <core/math/Base58.hpp>
#include <core/utils/Exception.hpp>

#include <bitcoin/BitcoinLikeAddress.hpp>
#include <bitcoin/BitcoinNetworks.hpp>
#include <bitcoin/bech32/Bech32.hpp>
#include <bitcoin/bech32/Bech32Factory.hpp>
#include <bitcoin/scripts/BitcoinLikeScriptOperators.hpp>


namespace ledger {
    namespace core {
        BitcoinLikeAddress::BitcoinLikeAddress(const ledger::core::api::Currency &currency,
                                               const std::vector<uint8_t>& hash160,
                                               const std::string &keychainEngine,
                                               const Option<std::string>& derivationPath) :
                _params(networks::getBitcoinLikeNetworkParameters(currency.name)),
                _derivationPath(derivationPath),
                _keychainEngine(keychainEngine),
                _hash160(hash160),
                Address(currency, derivationPath)
        {

        }

        std::vector<uint8_t> BitcoinLikeAddress::getVersion() {
            return getVersionFromKeychainEngine(_keychainEngine, _params);
        }

        std::vector<uint8_t> BitcoinLikeAddress::getVersionFromKeychainEngine(const std::string &keychainEngine, 
                                                                              const api::BitcoinLikeNetworkParameters &params) const {

            if (keychainEngine == api::KeychainEngines::BIP32_P2PKH) {
                return params.P2PKHVersion;
            } else if (keychainEngine == api::KeychainEngines::BIP49_P2SH) {
                return params.P2SHVersion;
            } else if (keychainEngine == api::KeychainEngines::BIP173_P2WPKH) {
                auto bech32Params = Bech32Parameters::getBech32Params(params.Identifier);
                return bech32Params.P2WPKHVersion;
            } else if (keychainEngine == api::KeychainEngines::BIP173_P2WSH) {
                auto bech32Params = Bech32Parameters::getBech32Params(params.Identifier);
                return bech32Params.P2WSHVersion;
            }
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid Keychain Engine: ", keychainEngine);
        };

        std::vector<uint8_t> BitcoinLikeAddress::getHash160() {
            return _hash160;
        }

        ledger::core::api::BitcoinLikeNetworkParameters BitcoinLikeAddress::getNetworkParameters() {
            return _params;
        }

        std::string BitcoinLikeAddress::toBase58() {
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", _params.Identifier);
            return Base58::encodeWithChecksum(vector::concat(getVersionFromKeychainEngine(_keychainEngine, _params), _hash160), config);
        }

        std::string toBech32Helper(const std::string &keychainEngine,
                                   const std::vector<uint8_t> &hash160,
                                   const api::BitcoinLikeNetworkParameters &params) {

            auto bech32 = Bech32Factory::newBech32Instance(params.Identifier).getValue();
            auto witnessVersion = (keychainEngine == api::KeychainEngines::BIP173_P2WPKH) ? bech32->getBech32Params().P2WPKHVersion : bech32->getBech32Params().P2WSHVersion;
            return bech32->encode(hash160, witnessVersion);
        }

        std::string BitcoinLikeAddress::toBech32() {
            return toBech32Helper(_keychainEngine, _hash160, _params);
        }

        bool BitcoinLikeAddress::isP2SH() {
            return _keychainEngine == api::KeychainEngines::BIP49_P2SH;
        }

        bool BitcoinLikeAddress::isP2PKH() {
            return _keychainEngine == api::KeychainEngines::BIP32_P2PKH;
        }

        bool BitcoinLikeAddress::isP2WSH() {
            return _keychainEngine == api::KeychainEngines::BIP173_P2WSH;
        }

        bool BitcoinLikeAddress::isP2WPKH() {
            return _keychainEngine == api::KeychainEngines::BIP173_P2WPKH;
        }

        std::experimental::optional<std::string> BitcoinLikeAddress::getDerivationPath() {
            return _derivationPath.toOptional();
        }

        std::string BitcoinLikeAddress::toBase58() const {
            if (_keychainEngine != api::KeychainEngines::BIP32_P2PKH && _keychainEngine != api::KeychainEngines::BIP49_P2SH) {
                throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Base58 format only available for api::KeychainEngines::BIP32_P2PKH and api::KeychainEngines::BIP49_P2SH");
            }
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", _params.Identifier);
            return Base58::encodeWithChecksum(vector::concat(getVersionFromKeychainEngine(_keychainEngine, _params), _hash160), config);
        }

        std::string BitcoinLikeAddress::toBech32() const {
            return toBech32Helper(_keychainEngine, _hash160, _params);
        }

        std::shared_ptr<Address>
        BitcoinLikeAddress::parse(const std::string &address,
                                  const ledger::core::api::Currency &currency,
                                  const Option<std::string>& derivationPath) {
            auto result = Try<std::shared_ptr<Address>>::from([&] () {
                auto bech32 = Bech32Factory::newBech32Instance(networks::getBitcoinLikeNetworkParameters(currency.name).Identifier);
                auto isBech32 = bech32.hasValue() && (bech32.getValue()->getBech32Params().hrp == address.substr(0, bech32.getValue()->getBech32Params().hrp.size()));
                return isBech32 ? fromBech32(address, currency, derivationPath) : fromBase58(address, currency, derivationPath);
            });
            return std::dynamic_pointer_cast<Address>(result.toOption().getValueOr(nullptr));
        }

        std::string BitcoinLikeAddress::toString() {
            return getStringAddress();
        }

        std::string BitcoinLikeAddress::getStringAddress() const {
            if (_keychainEngine == api::KeychainEngines::BIP173_P2WPKH || _keychainEngine == api::KeychainEngines::BIP173_P2WSH) {
                return toBech32();
            }
            return toBase58();
        }

        std::shared_ptr<BitcoinLikeAddress> BitcoinLikeAddress::fromBase58(const std::string &address,
                                                                           const api::Currency &currency,
                                                                           const Option<std::string>& derivationPath) {
            auto& params = networks::getBitcoinLikeNetworkParameters(currency.name);
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", params.Identifier);
            auto decoded = Base58::checkAndDecode(address, config);
            if (decoded.isFailure()) {
                throw decoded.getFailure();
            }
            auto value = decoded.getValue();

            //Check decoded address size
            if (value.size() <= 20) {
                throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Invalid address : Invalid base 58 format");
            }

            std::vector<uint8_t> hash160(value.end() - 20, value.end());
            std::vector<uint8_t> version(value.begin(), value.end() - 20);
            if (version != params.P2PKHVersion && version != params.P2SHVersion) {
                throw Exception(api::ErrorCode::INVALID_VERSION, "Address version doesn't belong to the given network parameters");
            }
            auto keychainEngine = (version == params.P2PKHVersion) ? api::KeychainEngines::BIP32_P2PKH : api::KeychainEngines::BIP49_P2SH;
            return std::make_shared<ledger::core::BitcoinLikeAddress>(currency, hash160, keychainEngine, derivationPath);
        }

        std::shared_ptr<BitcoinLikeAddress> BitcoinLikeAddress::fromBech32(const std::string& address,
                                                                           const api::Currency& currency,
                                                                           const Option<std::string>& derivationPath) {
            auto& params = networks::getBitcoinLikeNetworkParameters(currency.name);
            auto bech32 = Bech32Factory::newBech32Instance(params.Identifier).getValue();
            auto decoded = bech32->decode(address);
            auto keychainEngine = (decoded.second.size() == 32 || decoded.first == bech32->getBech32Params().P2WSHVersion) ? api::KeychainEngines::BIP173_P2WSH : api::KeychainEngines::BIP173_P2WPKH;
            return std::make_shared<ledger::core::BitcoinLikeAddress>(currency,
                                                                      decoded.second,
                                                                      keychainEngine,
                                                                      derivationPath);
        }

        std::string BitcoinLikeAddress::fromPublicKey(const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &pubKey,
                                                      const api::Currency &currency,
                                                      const std::string &derivationPath,
                                                      const std::string &keychainEngine) {
            if (keychainEngine == api::KeychainEngines::BIP32_P2PKH || keychainEngine == api::KeychainEngines::BIP173_P2WPKH) {
                return std::dynamic_pointer_cast<BitcoinLikeAddress>(pubKey->derive(derivationPath))->toString();
            } else if (keychainEngine == api::KeychainEngines::BIP49_P2SH || keychainEngine == api::KeychainEngines::BIP173_P2WSH) {
                auto hash160 = fromPublicKeyToHash160(pubKey->derivePublicKey(derivationPath), pubKey->deriveHash160(derivationPath), currency, keychainEngine);
                return BitcoinLikeAddress(currency, hash160, keychainEngine).toString();

            }
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid Keychain Engine: ", keychainEngine);
        }

        std::vector<uint8_t> BitcoinLikeAddress::fromPublicKeyToHash160(const std::vector<uint8_t> &pubKey,
                                                                        const std::vector<uint8_t> &pubKeyHash160,
                                                                        const api::Currency &currency,
                                                                        const std::string &keychainEngine) {
            if (keychainEngine == api::KeychainEngines::BIP49_P2SH) {
                //Script
                std::vector<uint8_t> script = {0x00, 0x14};
                //Insert hash160 of public key
                script.insert(script.end(), pubKeyHash160.begin(), pubKeyHash160.end());
                //Hash script
                HashAlgorithm hashAlgorithm(networks::getBitcoinLikeNetworkParameters(currency.name).Identifier);
                return HASH160::hash(script, hashAlgorithm);
            } else if (keychainEngine == api::KeychainEngines::BIP173_P2WSH) {
                // Reference https://bitcoincore.org/en/segwit_wallet_dev
                // Here scriptHash = SHA256(witnessScript) = SHA256(pubKeyHash160 + OP_CHECKSIG)
                const auto& params = networks::getBitcoinLikeNetworkParameters(currency.name);
                std::vector<uint8_t> witnessScript;
                //Hash160 of public key
                witnessScript.insert(witnessScript.end(), pubKey.begin(), pubKey.end());
                witnessScript.push_back(btccore::OP_CHECKSIG);
                return SHA256::bytesToBytesHash(witnessScript);
            }
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid Keychain Engine: ", keychainEngine);
        }

        std::vector<uint8_t> BitcoinLikeAddress::fromPublicKeyToHash160(const std::vector<uint8_t> &pubKey,
                                                                        const api::Currency &currency,
                                                                        const std::string &keychainEngine) {
            HashAlgorithm hashAlgorithm(networks::getBitcoinLikeNetworkParameters(currency.name).Identifier);
            auto publicKeyHash160 = HASH160::hash(pubKey, hashAlgorithm);
            if (keychainEngine == api::KeychainEngines::BIP32_P2PKH || keychainEngine == api::KeychainEngines::BIP173_P2WPKH) {
                return publicKeyHash160;
            } else if (keychainEngine == api::KeychainEngines::BIP49_P2SH || keychainEngine == api::KeychainEngines::BIP173_P2WSH) {
                return fromPublicKeyToHash160(pubKey, publicKeyHash160, currency, keychainEngine);
            }
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid Keychain Engine: ", keychainEngine);
        }
    }
}