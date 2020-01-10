/*
 *
 * RippleLikeKeychain
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

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/set.hpp>

#include <core/api/Currency.hpp>
#include <core/api/KeychainEngines.hpp>

#include <ripple/keychains/RippleLikeKeychain.hpp>
#include <ripple/RippleLikeExtendedPublicKey.hpp>
#include <ripple/RippleNetworks.hpp>

namespace ledger {
    namespace core {
        RippleLikeKeychain::RippleLikeKeychain(
            const std::shared_ptr<api::DynamicObject> &configuration,
            const api::Currency &params, int account,
            const std::shared_ptr<Preferences> &preferences
        ) :
            _account(account),
            _preferences(preferences),
            _configuration(configuration),
            _currency(params),
            _fullScheme(DerivationScheme(configuration
                        ->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME)
                        .value_or("44'/<coin_type>'/<account>'/<node>/<address>")
            )),
            _scheme(DerivationScheme(configuration
                        ->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME)
                        .value_or(
                            "44'/<coin_type>'/<account>'/<node>/<address>")
            ).getSchemeFrom(DerivationSchemeLevel::ACCOUNT_INDEX).shift()) {
        }

        RippleLikeKeychain::RippleLikeKeychain(
            const std::shared_ptr<api::DynamicObject> &configuration,
            const api::Currency &params,
            int account,
            const std::shared_ptr<api::RippleLikeExtendedPublicKey> &xpub,
            const std::shared_ptr<Preferences> &preferences
        ) : RippleLikeKeychain(configuration, params, account, preferences) {
            _xpub = xpub;
            getAllObservableAddresses(0, 0);
        }

        RippleLikeKeychain::RippleLikeKeychain(
            const std::shared_ptr<api::DynamicObject> &configuration,
            const api::Currency &params,
            int account,
            const std::string &accountAddress,
            const std::shared_ptr<Preferences> &preferences
        ) : RippleLikeKeychain(configuration, params, account, preferences) {
        }

        int RippleLikeKeychain::getAccountIndex() const {
            return _account;
        }

        const api::RippleLikeNetworkParameters RippleLikeKeychain::getNetworkParameters() const {
            return networks::getRippleLikeNetworkParameters(_currency.name);
        }


        std::shared_ptr<Preferences> RippleLikeKeychain::getPreferences() const {
            return _preferences;
        }

        std::shared_ptr<api::DynamicObject> RippleLikeKeychain::getConfiguration() const {
            return _configuration;
        }

        const api::Currency &RippleLikeKeychain::getCurrency() const {
            return _currency;
        }

        const DerivationScheme &RippleLikeKeychain::getDerivationScheme() const {
            return _scheme;
        }

        DerivationScheme &RippleLikeKeychain::getDerivationScheme() {
            return _scheme;
        }

        const DerivationScheme &RippleLikeKeychain::getFullDerivationScheme() const {
            return _fullScheme;
        }


        std::shared_ptr<RippleLikeAddress> RippleLikeKeychain::getAddress() const {
            if (_address.empty()) {
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, fmt::format("Address not derived yet from keychain"));
            }
            return std::dynamic_pointer_cast<RippleLikeAddress>(
                    RippleLikeAddress::parse(_address, getCurrency(), Option<std::string>(_localPath)));
        }

        Option<std::string> RippleLikeKeychain::getAddressDerivationPath(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                return Option<std::string>();
            } else {
                auto derivation = DerivationPath(getExtendedPublicKey()->getRootPath()) + DerivationPath(path);
                return Option<std::string>(derivation.toString());
            }
        }

        std::vector<RippleLikeKeychain::Address>
        RippleLikeKeychain::getAllObservableAddresses(uint32_t from, uint32_t to) {
            std::vector<RippleLikeKeychain::Address> result;
            result.push_back(derive());
            return result;
        }

        std::shared_ptr<api::RippleLikeExtendedPublicKey> RippleLikeKeychain::getExtendedPublicKey() const {
            return _xpub;
        }

        std::string RippleLikeKeychain::getRestoreKey() const {
            return _xpub->toBase58();
        }

        bool RippleLikeKeychain::contains(const std::string &address) const {
            return getAddressDerivationPath(address).nonEmpty();
        }

        int32_t RippleLikeKeychain::getOutputSizeAsSignedTxInput() const {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                                 "RippleLikeKeychain::getOutputSizeAsSignedTxInput is not implemented yet");
        }

        Option<std::vector<uint8_t>> RippleLikeKeychain::getPublicKey(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                Option<std::vector<uint8_t>>();
            }
            return Option<std::vector<uint8_t>>(_xpub->derivePublicKey(path));
        }

        RippleLikeKeychain::Address RippleLikeKeychain::derive() {
            if (_address.empty()) {
                _localPath = getDerivationScheme()
                    .setCoinType(getCurrency().bip44CoinType)
                    .getPath().toString();

                auto cacheKey = fmt::format("path:{}", _localPath);
                _address = getPreferences()->getString(cacheKey, "");
                if (_address.empty()) {
                    auto nodeScheme = getDerivationScheme()
                        .getSchemeFrom(DerivationSchemeLevel::NODE);
                    auto p = nodeScheme.getPath().getDepth() > 0 ? nodeScheme
                        .shift(1)
                        .setCoinType(getCurrency().bip44CoinType)
                        .getPath()
                        .toString() : "";

                    auto localNodeScheme = getDerivationScheme()
                        .getSchemeTo(DerivationSchemeLevel::NODE)
                        .setCoinType(getCurrency().bip44CoinType);
                    // If node level is hardened we don't derive according to it since private
                    // derivation are not supported
                    auto xpub = localNodeScheme.getPath().getDepth() > 0 && localNodeScheme.getPath().isHardened(0) ?
                        std::static_pointer_cast<RippleLikeExtendedPublicKey>(_xpub)->derive(DerivationPath("")) :
                        std::static_pointer_cast<RippleLikeExtendedPublicKey>(_xpub)->derive(localNodeScheme.getPath());

                    auto strScheme = localNodeScheme.getPath().toString();

                    _address = xpub->derive(p)->toBase58();
                    // Feed path -> address cache
                    // Feed address -> path cache
                    getPreferences()
                        ->edit()
                        ->putString(cacheKey, _address)
                        ->putString(fmt::format("address:{}", _address), _localPath)
                        ->commit();
                }
            }

            auto rippleAddress = RippleLikeAddress::parse(_address, getCurrency(), Option<std::string>(_localPath));
            if (!rippleAddress) {
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Could not derive address");
            }
            return std::dynamic_pointer_cast<RippleLikeAddress>(rippleAddress);
        }
    }
}
