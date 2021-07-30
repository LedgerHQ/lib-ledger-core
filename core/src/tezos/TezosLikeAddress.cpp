/*
 *
 * TezosLikeAddress
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

#include "TezosLikeAddress.h"
#include "TezosKey.h"
#include <utils/Exception.hpp>
#include <math/Base58.hpp>
#include <collections/vector.hpp>
#include <utils/hex.h>
#include <collections/DynamicObject.hpp>
#include <crypto/BLAKE.h>

namespace ledger {
    namespace core {
        TezosLikeAddress::TezosLikeAddress(const api::Currency &currency,
                                           const std::vector<uint8_t> &hash160,
                                           const std::vector<uint8_t> &version,
                                           const Option<std::string> &derivationPath) :
                AbstractAddress(currency, derivationPath),
                _hash160(hash160),
                _version(version),
                _params(currency.tezosLikeNetworkParameters.value()),
                _derivationPath(derivationPath)
        {}

        TezosLikeAddress::TezosLikeAddress(const api::Currency &currency,
                                           const std::vector<uint8_t> &pubKey,
                                           const std::vector<uint8_t> &version,
                                           api::TezosCurve curve,
                                           const Option<std::string> &derivationPath) :
                TezosLikeAddress(currency,
                                 getPublicKeyHash160(pubKey, curve),
                                 getPrefixFromImplicitVersion(version, curve),
                                 derivationPath)
        {}

        std::vector<uint8_t> TezosLikeAddress::getVersion() {
            return _version;
        }

        std::vector<uint8_t> TezosLikeAddress::getHash160() {
            return _hash160;
        }

        api::TezosLikeNetworkParameters TezosLikeAddress::getNetworkParameters() {
            return _params;
        }

        std::string TezosLikeAddress::toBase58() {
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", _params.Identifier);
            return Base58::encodeWithChecksum(vector::concat(getVersion(), _hash160), config);
        }

        std::experimental::optional<std::string> TezosLikeAddress::getDerivationPath() {
            return _derivationPath.toOptional();
        }

        std::string TezosLikeAddress::toString() {
            return toBase58();
        }

        const std::vector<std::string> TEZOS_PREFIXES = {"tz1", "tz2", "tz3", "KT1"};

        std::shared_ptr<AbstractAddress>
        TezosLikeAddress::parse(const std::string &address,
                                const api::Currency &currency,
                                const Option<std::string> &derivationPath) {

            auto hasValidPrefix = [address](std::string prefix) {
                return address.rfind(prefix, 0) == 0;
            };
            if (std::none_of(std::begin(TEZOS_PREFIXES), std::end(TEZOS_PREFIXES), hasValidPrefix)) {
                return nullptr;
            }
            auto result = Try<std::shared_ptr<ledger::core::AbstractAddress>>::from([&]() {
                return fromBase58(address, currency, derivationPath);
            });
            return std::dynamic_pointer_cast<AbstractAddress>(result.toOption().getValueOr(nullptr));
        }

        std::shared_ptr<TezosLikeAddress> TezosLikeAddress::fromBase58(const std::string &address,
                                                                       const api::Currency &currency,
                                                                       const Option<std::string> &derivationPath) {
            std::vector<uint8_t> hash160, version;
            if (!address.empty()) {
                auto &params = currency.tezosLikeNetworkParameters.value();
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

                hash160 = std::vector<uint8_t>(value.end() - 20, value.end());
                version = std::vector<uint8_t>(value.begin(), value.end() - 20);
            }
            return std::make_shared<ledger::core::TezosLikeAddress>(currency, hash160, version, derivationPath);
        }

        // Reference: https://gitlab.com/tezos/tezos/blob/952dacac820e337577d5a6ea8db881883d9d865c/src/lib_signer_backends/ledger.ml#L100
        std::vector<uint8_t> TezosLikeAddress::getPublicKeyHash160(
            const std::vector<uint8_t> &pubKey,
            api::TezosCurve curve
        ) {
            // std::string out;
            // out.assign(pubKey.begin(), pubKey.end());
            // auto out = std::string str(pubKey.begin(), pubKey.end());
            switch (curve) {
                case api::TezosCurve::ED25519 : {
                    if (pubKey.size() == TezosKeyType::EDPK.length) {
                        return BLAKE::blake2b(pubKey, 20);
                    }
					else if (pubKey.size() == TezosKeyType::XPUB.length) {
						return BLAKE::blake2b(std::vector<uint8_t>{pubKey.begin() + 1, pubKey.end()}, 20);
					}
                    std::string msg = "Invalid ED25519 public key: should be "
                        + std::to_string(TezosKeyType::EDPK.length)
                        + " or " + std::to_string(TezosKeyType::XPUB.length)
                        + " bytes.";
                    throw Exception(api::ErrorCode::INVALID_ARGUMENT, msg);
                }

                case api::TezosCurve::SECP256K1: {
                    if (pubKey.size() == TezosKeyType::SPPK.length) {
                        return BLAKE::blake2b(
                            vector::concat(
                                std::vector<uint8_t>{static_cast<uint8_t>(pubKey[00])},
                                std::vector<uint8_t>{pubKey.begin() + 1, pubKey.begin() + 33}
                            ),
                            20
                        );
                    }
                    else if (pubKey.size() == 65) {
                        return BLAKE::blake2b(
                            vector::concat(
                                    std::vector<uint8_t>{static_cast<uint8_t>(0x02 + (pubKey[64] & 0x01))},
                                    std::vector<uint8_t>{pubKey.begin() + 1, pubKey.begin() + 33}
                            ),
                            20
                        );
                    }

                    std::string msg = "Invalid SECP256K1 public key: should be ";
                    msg += std::to_string(TezosKeyType::SPPK.length);
                    msg += " or 65 bytes.";
                    throw Exception(api::ErrorCode::INVALID_ARGUMENT, msg);
                }
                case api::TezosCurve::P256: {
                    if (pubKey.size() == TezosKeyType::P2PK.length) {
                        return BLAKE::blake2b(
                            vector::concat(
                                std::vector<uint8_t>{static_cast<uint8_t>(pubKey[00])},
                                std::vector<uint8_t>{pubKey.begin() + 1, pubKey.begin() + 33}
                            ),
                            20
                        );
                    }
                    else if (pubKey.size() == 65) {
                        return BLAKE::blake2b(
                            vector::concat(
                                    std::vector<uint8_t>{static_cast<uint8_t>(0x02 + (pubKey[64] & 0x01))},
                                    std::vector<uint8_t>{pubKey.begin() + 1, pubKey.begin() + 33}
                            ),
                            20
                        );
                    }
                    std::string msg = "Invalid P256 public key: should be ";
                    msg += std::to_string(TezosKeyType::P2PK.length);
                    msg += " or 65 bytes.";
                    throw Exception(api::ErrorCode::INVALID_ARGUMENT, msg);
                }
                default : {
                    throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid public key type.");
                }
            }
        }

        std::vector<uint8_t> TezosLikeAddress::getPrefixFromImplicitVersion(const std::vector<uint8_t> &implicitVersion,
                                                                            api::TezosCurve curve) {
            return vector::concat(
                    std::vector<uint8_t>{implicitVersion.begin(), implicitVersion.end() - 1},
                    std::vector<uint8_t>{static_cast<uint8_t>(implicitVersion.back() + static_cast<uint8_t>(
                            curve == api::TezosCurve::SECP256K1 ? 2 :
                            curve == api::TezosCurve::P256 ? 5 :
                            0)
                    )}
            );
        }
    }
}
