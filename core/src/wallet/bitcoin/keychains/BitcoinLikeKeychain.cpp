/*
 *
 * BitcoinLikeKeychain
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/01/2017.
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

#include <api/Currency.hpp>
#include "BitcoinLikeKeychain.hpp"
#include <api/KeychainEngines.hpp>
namespace ledger {
    namespace core {

        int BitcoinLikeKeychain::getAccountIndex() const {
            return _account;
        }

        const api::BitcoinLikeNetworkParameters &BitcoinLikeKeychain::getNetworkParameters() const {
            return _currency.bitcoinLikeNetworkParameters.value();
        }

        bool BitcoinLikeKeychain::markAsUsed(const std::vector<std::string> &addresses) {
            bool result = false;
            for (auto& address : addresses) {
                result = markAsUsed(address) || result;
            }
            return result;
        }

        std::shared_ptr<Preferences> BitcoinLikeKeychain::getPreferences() const {
            return _preferences;
        }

        std::shared_ptr<api::DynamicObject> BitcoinLikeKeychain::getConfiguration() const {
            return _configuration;
        }

        BitcoinLikeKeychain::BitcoinLikeKeychain(const std::shared_ptr<api::DynamicObject>& configuration,
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

        const api::Currency& BitcoinLikeKeychain::getCurrency() const {
            return _currency;
        }

        const DerivationScheme &BitcoinLikeKeychain::getDerivationScheme() const {
            return _scheme;
        }

        DerivationScheme &BitcoinLikeKeychain::getDerivationScheme() {
            return _scheme;
        }

        bool BitcoinLikeKeychain::markAsUsed(const std::string &address) {
            auto path = getAddressDerivationPath(address);
            if (path.nonEmpty()) {
                return markPathAsUsed(DerivationPath(path.getValue()));
            } else {
                return false;
            }
        }

        const DerivationScheme &BitcoinLikeKeychain::getFullDerivationScheme() const {
            return _fullScheme;
        }

        bool BitcoinLikeKeychain::isSegwit() const {
            //TODO: find a better way to know whether it's segwit or not
            auto keychainEngine = _configuration->getString(api::Configuration::KEYCHAIN_ENGINE).value_or(api::KeychainEngines::BIP32_P2PKH);
            return (keychainEngine == api::KeychainEngines::BIP49_P2SH ||
                    keychainEngine == api::KeychainEngines::BIP173_P2WPKH ||
                    keychainEngine == api::KeychainEngines::BIP173_P2WSH) ? true : false;
        }
    }
}