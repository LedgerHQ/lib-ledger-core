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

#include <api/Currency.hpp>
#include "EthereumLikeKeychain.hpp"
#include <api/KeychainEngines.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/set.hpp>
#include <ethereum/EthereumLikeExtendedPublicKey.h>

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

            // Try to restore the state from preferences
            auto state = preferences->getData("state", {});
            if (!state.empty()) {
                boost::iostreams::array_source my_vec_source(reinterpret_cast<char*>(&state[0]), state.size());
                boost::iostreams::stream<boost::iostreams::array_source> is(my_vec_source);
                ::cereal::BinaryInputArchive archive(is);
                archive(_state);
            } else {
                _state.maxConsecutiveIndex = 0;
                _state.empty = true;
            }
            _observableRange = (uint32_t) configuration->getInt(api::Configuration::KEYCHAIN_OBSERVABLE_RANGE).value_or(20);

            getAllObservableAddresses(0, _observableRange);

        }
        
        int EthereumLikeKeychain::getAccountIndex() const {
            return _account;
        }

        const api::EthereumLikeNetworkParameters &EthereumLikeKeychain::getNetworkParameters() const {
            return _currency.ethereumLikeNetworkParameters.value();
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

        EthereumLikeKeychain::Address EthereumLikeKeychain::getFreshAddress() {
            return derive(_state.maxConsecutiveIndex);
        }

        bool EthereumLikeKeychain::isEmpty() const {
            return _state.empty;
        }

        std::vector<EthereumLikeKeychain::Address>
        EthereumLikeKeychain::getFreshAddresses(size_t n) {
            auto startOffset = _state.maxConsecutiveIndex;
            std::vector<EthereumLikeKeychain::Address> result(n);
            for (auto i = 0; i < n; i++) {
                result[i] = derive(startOffset + i);
            }
            return result;
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
            auto maxObservableIndex = _state.maxConsecutiveIndex + _state.nonConsecutiveIndexes.size() + _observableRange;
            auto length = std::min<size_t >(to - from, maxObservableIndex - from);
            std::vector<EthereumLikeKeychain::Address> result;
            for (auto i = 0; i <= length; i++) {
                result.push_back(derive(from + i));
            }
            return result;
        }

        void EthereumLikeKeychain::saveState() {

            while (_state.nonConsecutiveIndexes.find(_state.maxConsecutiveIndex) != _state.nonConsecutiveIndexes.end()) {
                _state.nonConsecutiveIndexes.erase(_state.maxConsecutiveIndex);
                _state.maxConsecutiveIndex += 1;
            }

            std::stringstream is;
            ::cereal::BinaryOutputArchive archive(is);
            archive(_state);
            auto savedState = is.str();
            getPreferences()->edit()->putData("state", std::vector<uint8_t>((const uint8_t *)savedState.data(),(const uint8_t *)savedState.data() + savedState.size()))->commit();
        }

        std::shared_ptr<api::EthereumLikeExtendedPublicKey> EthereumLikeKeychain::getExtendedPublicKey() const {
            return _xpub;
        }

        std::string EthereumLikeKeychain::getRestoreKey() const {
            return _xpub->toBase58();
        }

        int32_t EthereumLikeKeychain::getObservableRangeSize() const {
            return _observableRange;
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

        EthereumLikeKeychain::Address EthereumLikeKeychain::derive(off_t index) {

            auto localPath = getDerivationScheme()
                    .setAccountIndex(getAccountIndex())
                    .setCoinType(getCurrency().bip44CoinType)
                    .setNode(0)
                    .setAddressIndex((int) index).getPath().toString();
            auto cacheKey = fmt::format("path:{}", localPath);
            auto address = getPreferences()->getString(cacheKey, "");
            if (address.empty()) {
                auto p = getDerivationScheme().getSchemeFrom(DerivationSchemeLevel::NODE).shift(1)
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(getCurrency().bip44CoinType)
                        .setNode(0)
                        .setAddressIndex((int) index).getPath().toString();
                auto xpub = _xpub;
                address = xpub->derive(p)->toBase58();
                // Feed path -> address cache
                // Feed address -> path cache
                getPreferences()
                        ->edit()
                        ->putString(cacheKey, address)
                        ->putString(fmt::format("address:{}", address), localPath)
                        ->commit();
            }
            auto abstractAddress = EthereumLikeAddress::parse(address, getCurrency(), Option<std::string>(localPath));
            return std::dynamic_pointer_cast<EthereumLikeAddress>(abstractAddress);
        }


    }
}