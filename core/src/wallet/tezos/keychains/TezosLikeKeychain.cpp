/*
 *
 * TezosLikeKeychain
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#include "TezosLikeKeychain.h"
#include <api/Currency.hpp>
#include <api/KeychainEngines.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/set.hpp>
#include <tezos/TezosLikeExtendedPublicKey.h>
namespace ledger {
    namespace core {
        TezosLikeKeychain::TezosLikeKeychain(const std::shared_ptr <api::DynamicObject> &configuration,
                                             const api::Currency &params, int account,
                                             const std::shared_ptr <Preferences> &preferences) :
                _account(account), _preferences(preferences), _configuration(configuration), _currency(params),
                _fullScheme(DerivationScheme(configuration
                                                     ->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME)
                                                     .value_or("44'/<coin_type>'/<account>'/<node>/<address>"))),
                _scheme(DerivationScheme(configuration
                                                 ->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME)
                                                 .value_or(
                                                         "44'/<coin_type>'/<account>'/<node>/<address>")).getSchemeFrom(
                        DerivationSchemeLevel::ACCOUNT_INDEX).shift()) {
        }

        TezosLikeKeychain::TezosLikeKeychain(const std::shared_ptr <api::DynamicObject> &configuration,
                                             const api::Currency &params,
                                             int account,
                                             const std::shared_ptr <api::TezosLikeExtendedPublicKey> &xpub,
                                             const std::shared_ptr <Preferences> &preferences)
                : TezosLikeKeychain(configuration, params, account, preferences) {

            _xpub = xpub;
            getAllObservableAddresses(0, 0);
        }

        TezosLikeKeychain::TezosLikeKeychain(const std::shared_ptr <api::DynamicObject> &configuration,
                                             const api::Currency &params,
                                             int account,
                                             const std::string &accountAddress,
                                             const std::shared_ptr <Preferences> &preferences)
                : TezosLikeKeychain(configuration, params, account, preferences) {}

        int TezosLikeKeychain::getAccountIndex() const {
            return _account;
        }

        const api::TezosLikeNetworkParameters &TezosLikeKeychain::getNetworkParameters() const {
            return _currency.tezosLikeNetworkParameters.value();
        }


        std::shared_ptr <Preferences> TezosLikeKeychain::getPreferences() const {
            return _preferences;
        }

        std::shared_ptr <api::DynamicObject> TezosLikeKeychain::getConfiguration() const {
            return _configuration;
        }

        const api::Currency &TezosLikeKeychain::getCurrency() const {
            return _currency;
        }

        const DerivationScheme &TezosLikeKeychain::getDerivationScheme() const {
            return _scheme;
        }

        DerivationScheme &TezosLikeKeychain::getDerivationScheme() {
            return _scheme;
        }

        const DerivationScheme &TezosLikeKeychain::getFullDerivationScheme() const {
            return _fullScheme;
        }


        std::shared_ptr <TezosLikeAddress> TezosLikeKeychain::getAddress() const {
            if (_address.empty()) {
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, fmt::format("Address not derived yet from keychain"));
            }
            return std::dynamic_pointer_cast<TezosLikeAddress>(
                    TezosLikeAddress::parse(_address, getCurrency(), Option<std::string>(_localPath)));
        }

        Option <std::string> TezosLikeKeychain::getAddressDerivationPath(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                return Option<std::string>();
            } else {
                auto derivation = DerivationPath(getExtendedPublicKey()->getRootPath()) + DerivationPath(path);
                return Option<std::string>(derivation.toString());
            }
        }

        std::vector <TezosLikeKeychain::Address>
        TezosLikeKeychain::getAllObservableAddresses(uint32_t from, uint32_t to) {
            std::vector <TezosLikeKeychain::Address> result;
            result.push_back(derive());
            return result;
        }


        std::shared_ptr <api::TezosLikeExtendedPublicKey> TezosLikeKeychain::getExtendedPublicKey() const {
            return _xpub;
        }

        std::string TezosLikeKeychain::getRestoreKey() const {
            return _xpub->toBase58();
        }


        bool TezosLikeKeychain::contains(const std::string &address) const {
            return getAddressDerivationPath(address).nonEmpty();
        }

        int32_t TezosLikeKeychain::getOutputSizeAsSignedTxInput() const {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                                 "TezosLikeKeychain::getOutputSizeAsSignedTxInput is not implemented yet");
        }

        Option <std::vector<uint8_t>> TezosLikeKeychain::getPublicKey(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                Option <std::vector<uint8_t>> ();
            }
            return Option <std::vector<uint8_t>> (_xpub->derivePublicKey(path));
        }

        TezosLikeKeychain::Address TezosLikeKeychain::derive() {

            if (_address.empty()) {
                _localPath = getDerivationScheme()
                        .setCoinType(getCurrency().bip44CoinType)
                        .getPath().toString();

                auto cacheKey = fmt::format("path:{}", _localPath);
                _address = getPreferences()->getString(cacheKey, "");
                if (_address.empty()) {
                    auto p = getDerivationScheme().getSchemeFrom(DerivationSchemeLevel::NODE).shift(1)
                            .setCoinType(getCurrency().bip44CoinType)
                            .getPath().toString();
                    auto xpub = _xpub;
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

            auto tezosAddress = TezosLikeAddress::parse(_address, getCurrency(), Option<std::string>(_localPath));
            if (!tezosAddress) {
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Could not derive address");
            }
            return std::dynamic_pointer_cast<TezosLikeAddress>(tezosAddress);
        }
    }
}