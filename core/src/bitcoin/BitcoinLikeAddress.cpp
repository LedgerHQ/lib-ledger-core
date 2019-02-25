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
#include "BitcoinLikeAddress.hpp"
#include "../utils/djinni_helpers.hpp"
#include "../bytes/BytesWriter.h"
#include "../math/Base58.hpp"
#include "../collections/vector.hpp"
#include "../utils/Exception.hpp"
#include <collections/DynamicObject.hpp>
#include <bitcoin/bech32/Bech32.h>
#include <bitcoin/bech32/Bech32Factory.h>
#include <api/KeychainEngines.hpp>
#include <crypto/HashAlgorithm.h>
#include <crypto/HASH160.hpp>
#include <wallet/bitcoin/scripts/operators.h>

namespace ledger {
    namespace core {
        BitcoinLikeAddress::BitcoinLikeAddress(const ledger::core::api::Currency &currency,
                                               const std::vector<uint8_t>& hash160,
                                               const std::vector<uint8_t>& version,
                                               const Option<std::string>& derivationPath) :
                _params(currency.bitcoinLikeNetworkParameters.value()),
                _version(version),
                _derivationPath(derivationPath),
                _hash160(hash160),
                AbstractAddress(currency, derivationPath)
        {

        }

        std::vector<uint8_t> BitcoinLikeAddress::getVersion() {
            return _version;
        }

        std::vector<uint8_t> BitcoinLikeAddress::getHash160() {
            return _hash160;
        }

        ledger::core::api::BitcoinLikeNetworkParameters BitcoinLikeAddress::getNetworkParameters() {
            return _params;
        }

        std::string BitcoinLikeAddress::toBase58() {
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", _params.Identifier);
            return Base58::encodeWithChecksum(vector::concat(_version, _hash160), config);
        }

        std::string toBech32Helper(const std::vector<uint8_t> &version,
                                   const std::vector<uint8_t> &hash160,
                                   const api::BitcoinLikeNetworkParameters &params) {
            auto bech32 = Bech32Factory::newBech32Instance(params.Identifier);
            auto witnessVersion = (version == params.P2PKHVersion) ? bech32->getBech32Params().P2WPKHVersion : bech32->getBech32Params().P2WSHVersion;
            return bech32->encode(hash160, witnessVersion);
        }

        std::string BitcoinLikeAddress::toBech32() {
            return toBech32Helper(_version, _hash160, _params);
        }

        bool BitcoinLikeAddress::isP2SH() {
            return _version == _params.P2SHVersion;
        }

        bool BitcoinLikeAddress::isP2PKH() {
            return _version == _params.P2PKHVersion;
        }

        std::experimental::optional<std::string> BitcoinLikeAddress::getDerivationPath() {
            return _derivationPath.toOptional();
        }

        std::string BitcoinLikeAddress::toBase58() const {
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", _params.Identifier);
            return Base58::encodeWithChecksum(vector::concat(_version, _hash160), config);
        }
        }

        std::string BitcoinLikeAddress::toBech32() const {
            return toBech32Helper(_version, _hash160, _params);
        }

        std::shared_ptr<ledger::core::AbstractAddress>
        BitcoinLikeAddress::parse(const std::string &address,
                                  const ledger::core::api::Currency &currency,
                                  const Option<std::string>& derivationPath) {
            auto result = Try<std::shared_ptr<ledger::core::AbstractAddress>>::from([&] () {
                auto bech32Hrp = Bech32Factory::newBech32Instance(currency.bitcoinLikeNetworkParameters.value().Identifier)->getBech32Params().hrp;
                auto isBech32 = bech32Hrp == address.substr(0, bech32Hrp.size());
                return isBech32 ? fromBech32(address, currency) : fromBase58(address, currency, derivationPath);
            });
            return std::dynamic_pointer_cast<AbstractAddress>(result.toOption().getValueOr(nullptr));
        }

        std::string BitcoinLikeAddress::toString() {
            return toBase58();
        }

        std::shared_ptr<BitcoinLikeAddress> BitcoinLikeAddress::fromBase58(const std::string &address,
                                                                           const api::Currency &currency,
                                                                           const Option<std::string>& derivationPath) {
            auto& params = currency.bitcoinLikeNetworkParameters.value();
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
            return std::make_shared<ledger::core::BitcoinLikeAddress>(currency, hash160, version, derivationPath);
        }

        std::shared_ptr<BitcoinLikeAddress> BitcoinLikeAddress::fromBech32(const std::string& address,
                                                                           const api::Currency& currency,
                                                                           const Option<std::string>& derivationPath) {
            auto& params = currency.bitcoinLikeNetworkParameters.value();
            auto bech32 = Bech32Factory::newBech32Instance(params.Identifier);
            auto decoded = bech32->decode(address);
            auto version = (decoded.second.size() == 32 || decoded.first == bech32->getBech32Params().P2WSHVersion) ? params.P2SHVersion : params.P2PKHVersion;
            return std::make_shared<ledger::core::BitcoinLikeAddress>(currency,
                                                                      decoded.second,
                                                                      version,
                                                                      derivationPath);
        }

        std::string BitcoinLikeAddress::fromPublicKey(const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &pubKey,
                                                      const api::Currency &currency,
                                                      const std::string &derivationPath,
                                                      const std::shared_ptr<DynamicObject> &keychainConfig) {
            //Get keychainEngine and its version
            auto keychainEngine = keychainConfig->getString("keychainEngines").value_or(api::KeychainEngines::BIP32_P2PKH);
            auto version = keychainConfig->getData("version").value_or(currency.bitcoinLikeNetworkParameters.value().P2PKHVersion);
            if (version.empty()) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Fail to retrieve address from public key: empty Script Version");
            }

            if (keychainEngine == api::KeychainEngines::BIP32_P2PKH) {
                return pubKey->derive(derivationPath)->toBase58();
            } else if (keychainEngine == api::KeychainEngines::BIP49_P2SH) {
                //Script
                std::vector<uint8_t> script = {0x00, 0x14};
                //Hash160 of public key
                auto publicKeyHash160 = pubKey->deriveHash160(derivationPath);
                script.insert(script.end(), publicKeyHash160.begin(), publicKeyHash160.end());
                //Hash script
                const auto& params = currency.bitcoinLikeNetworkParameters.value();
                HashAlgorithm hashAlgorithm(params.Identifier);
                auto hash160 = HASH160::hash(script, hashAlgorithm);
                BitcoinLikeAddress btcLikeAddress(currency, hash160, version);
                return btcLikeAddress.toBase58();
            } else if (keychainEngine == api::KeychainEngines::BIP173_P2WPKH) {
                return pubKey->derive(derivationPath)->toBech32();
            } else if (keychainEngine == api::KeychainEngines::BIP173_P2WSH) {
                // Reference https://bitcoincore.org/en/segwit_wallet_dev
                // Here scriptHash = SHA256(witnessScript) = SHA256(pubKeyHash160 + OP_CHECKSIG)
                const auto& params = currency.bitcoinLikeNetworkParameters.value();
                auto publicKey = pubKey->derivePublicKey(derivationPath);
                std::vector<uint8_t> witnessScript;
                //Hash160 of public key
                witnessScript.insert(witnessScript.end(), publicKey.begin(), publicKey.end());
                witnessScript.push_back(btccore::OP_CHECKSIG);
                auto scriptHash = SHA256::bytesToBytesHash(witnessScript);
                // Use P2SHVersion to be able to compute Base58 format
                BitcoinLikeAddress btcLikeAddress(currency, scriptHash, params.P2SHVersion);
                return btcLikeAddress.toBech32();

            }
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid Keychain Engine: ", keychainEngine);
        }
    }
}