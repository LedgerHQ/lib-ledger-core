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
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
namespace ledger {
    namespace core {

        TezosLikeKeychain::TezosLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
                                             const api::Currency &params,
                                             const Option<std::vector<uint8_t>> &pubKey,
                                             const std::shared_ptr<Preferences> &preferences)
                : _configuration(configuration), _currency(params), _publicKey(pubKey) {
            _address = getAddressFromPublicKey();
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

        std::shared_ptr <TezosLikeAddress> TezosLikeKeychain::getAddress() const {
            return _address;
        }

        std::vector <TezosLikeKeychain::Address>
        TezosLikeKeychain::getAllObservableAddresses(uint32_t from, uint32_t to) {
            return { getAddressFromPublicKey() };
        }

        std::string TezosLikeKeychain::getRestoreKey() const {
            return hex::toString(_publicKey.getValueOr(std::vector<uint8_t>()));
        }

        bool TezosLikeKeychain::contains(const std::string &address) const {
            return _address->toString() == address;
        }

        Option <std::vector<uint8_t>> TezosLikeKeychain::getPublicKey() const {
            return _publicKey;
        }

        TezosLikeKeychain::Address TezosLikeKeychain::getAddressFromPublicKey() {
            auto curveConfig = _configuration->getString(api::TezosConfiguration::TEZOS_XPUB_CURVE);
            api::TezosCurve curve;
            if (curveConfig == api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519) {
                curve = api::TezosCurve::ED25519;
            } else if (curveConfig == api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_SECP256K1) {
                curve = api::TezosCurve::SECP256K1;
            } else if (curveConfig == api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_P256) {
                curve = api::TezosCurve::P256;
            } else {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Could not retrieve address from public key : unknown Tezos Curve");
            }
            return std::make_shared<TezosLikeAddress>(_currency,
                                                      _publicKey.getValueOr(std::vector<uint8_t>()),
                                                      _currency.tezosLikeNetworkParameters.value().ImplicitPrefix,
                                                      curve,
                                                      Option<std::string>(""));
        }
    }
}