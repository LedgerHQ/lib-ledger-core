/*
 *
 * EthereumLikeKeychain
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

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/set.hpp>

#include <core/api/Currency.hpp>
#include <core/api/KeychainEngines.hpp>

#include <ethereum/keychains/EthereumLikeKeychain.hpp>
#include <ethereum/EthereumLikeExtendedPublicKey.hpp>
#include <ethereum/EthereumNetworks.hpp>

namespace ledger {
    namespace core {

        EthereumLikeKeychain::EthereumLikeKeychain(const std::shared_ptr<api::DynamicObject>& configuration,
                                                   const api::Currency &params, int account,
                                                   const std::shared_ptr<Preferences>& preferences) :
                _account(account), _preferences(preferences), _configuration(configuration), _currency(params),
                _fullScheme(DerivationScheme(configuration
                                                     ->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME)
                                                     .value_or("44'/<coin_type>'/<account>'/<node>/<address>"))),
                _scheme(DerivationScheme(configuration
                                                 ->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME)
                                                 .value_or("44'/<coin_type>'/<account>'/<node>/<address>")).getSchemeFrom(DerivationSchemeLevel::ACCOUNT_INDEX).shift())
        {
        }
        
        EthereumLikeKeychain::EthereumLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
                                                   const api::Currency &params,
                                                   int account,
                                                   const std::shared_ptr<api::EthereumLikeExtendedPublicKey> &xpub,
                                                   const std::shared_ptr<Preferences> &preferences)
                : EthereumLikeKeychain(configuration, params, account, preferences) {

            _xpub = xpub;
            getAllObservableAddresses(0,0);
        }

        EthereumLikeKeychain::EthereumLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
                                                   const api::Currency &params,
                                                   int account,
                                                   const std::string &accountAddress,
                                                   const std::shared_ptr<Preferences>& preferences)
                : EthereumLikeKeychain(configuration, params, account, preferences)
        {}
        
        int EthereumLikeKeychain::getAccountIndex() const {
            return _account;
        }

        const api::EthereumLikeNetworkParameters EthereumLikeKeychain::getNetworkParameters() const {
            return networks::getEthereumLikeNetworkParameters(_currency.name);
        }


        std::shared_ptr<Preferences> EthereumLikeKeychain::getPreferences() const {
            return _preferences;
        }

        std::shared_ptr<api::DynamicObject> EthereumLikeKeychain::getConfiguration() const {
            return _configuration;
        }

        const api::Currency& EthereumLikeKeychain::getCurrency() const {
            return _currency;
        }

        const DerivationScheme &EthereumLikeKeychain::getDerivationScheme() const {
            return _scheme;
        }

        DerivationScheme &EthereumLikeKeychain::getDerivationScheme() {
            return _scheme;
        }

        const DerivationScheme &EthereumLikeKeychain::getFullDerivationScheme() const {
            return _fullScheme;
        }


        std::shared_ptr<EthereumLikeAddress> EthereumLikeKeychain::getAddress() const {
            if (_address.empty()) {
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, fmt::format("Address not derived yet from keychain"));
            }
            return std::dynamic_pointer_cast<EthereumLikeAddress>(EthereumLikeAddress::parse(_address, getCurrency(), Option<std::string>(_localPath)));
        }

        Option<std::string> EthereumLikeKeychain::getAddressDerivationPath(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                return Option<std::string>();
            } else {
                auto derivation = DerivationPath(getExtendedPublicKey()->getRootPath()) + DerivationPath(path);
                return Option<std::string>(derivation.toString());
            }
        }

        std::vector<EthereumLikeKeychain::Address>
        EthereumLikeKeychain::getAllObservableAddresses(uint32_t from, uint32_t to) {
            std::vector<EthereumLikeKeychain::Address> result;
            result.push_back(derive());
            return result;
        }


        std::shared_ptr<api::EthereumLikeExtendedPublicKey> EthereumLikeKeychain::getExtendedPublicKey() const {
            return _xpub;
        }

        std::string EthereumLikeKeychain::getRestoreKey() const {
            return _xpub->toBase58();
        }


        bool EthereumLikeKeychain::contains(const std::string &address) const {
            return getAddressDerivationPath(address).nonEmpty();
        }

        int32_t EthereumLikeKeychain::getOutputSizeAsSignedTxInput() const {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeKeychain::getOutputSizeAsSignedTxInput is not implemented yet");
        }

        Option<std::vector<uint8_t>> EthereumLikeKeychain::getPublicKey(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                Option<std::vector<uint8_t>>();
            }
            return Option<std::vector<uint8_t>>(_xpub->derivePublicKey(path));
        }

        EthereumLikeKeychain::Address EthereumLikeKeychain::derive() {

            if (_address.empty()) {

                auto coinType = _fullScheme.getCoinType() ? _fullScheme.getCoinType() : getCurrency().bip44CoinType;
                _localPath = getDerivationScheme()
                        .setCoinType(coinType)
                        .getPath().toString();

                auto cacheKey = fmt::format("path:{}", _localPath);
                _address = getPreferences()->getString(cacheKey, "");
                if (_address.empty()) {
                    auto nodeScheme = getDerivationScheme()
                            .getSchemeFrom(DerivationSchemeLevel::NODE);
                    auto p = nodeScheme.getPath().getDepth() > 0 ? nodeScheme
                            .shift(1)
                            .setCoinType(coinType)
                            .getPath()
                            .toString() : "";

                    auto localNodeScheme = getDerivationScheme()
                            .getSchemeTo(DerivationSchemeLevel::NODE)
                            .setCoinType(coinType);
                    // If node level is hardened we don't derive according to it since private
                    // derivation are not supported 
                    auto xpub = localNodeScheme.getPath().getDepth() > 0 && localNodeScheme.getPath().isHardened(0) ? std::static_pointer_cast<EthereumLikeExtendedPublicKey>(_xpub)->derive(DerivationPath("")) : std::static_pointer_cast<EthereumLikeExtendedPublicKey>(_xpub)->derive(localNodeScheme.getPath());
                    auto strScheme = localNodeScheme.getPath().toString();

                    _address = xpub->derive(p)->toEIP55();
                    // Feed path -> address cache
                    // Feed address -> path cache
                    getPreferences()
                            ->edit()
                            ->putString(cacheKey, _address)
                            ->putString(fmt::format("address:{}", _address), _localPath)
                            ->commit();
                }
            }

            auto ethAddress = EthereumLikeAddress::parse(_address, getCurrency(), Option<std::string>(_localPath));
            if (!ethAddress) {
                throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Could not derive address");
            }
            return std::dynamic_pointer_cast<EthereumLikeAddress>(ethAddress);
        }


    }
}