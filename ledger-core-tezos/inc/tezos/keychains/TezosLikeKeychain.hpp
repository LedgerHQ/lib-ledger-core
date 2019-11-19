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

#pragma once 

#include <string>
#include <vector>

#include <tezos/TezosLikeAddress.hpp>
#include <tezos/api/TezosLikeExtendedPublicKey.hpp>

#include <core/api/AccountCreationInfo.hpp>
#include <core/api/Configuration.hpp>
#include <core/api/Currency.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/api/ExtendedKeyAccountCreationInfo.hpp>
#include <core/preferences/Preferences.hpp>
#include <core/utils/DerivationScheme.hpp>
#include <core/utils/Option.hpp>

namespace ledger {
    namespace core {
        class TezosLikeKeychain {

        public:
            using Address = std::shared_ptr<TezosLikeAddress>;

            TezosLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
                              const api::Currency &params,
                              const Option<std::vector<uint8_t>> &pubKey,
                              const std::shared_ptr<Preferences> &preferences);

            std::vector <Address> getAllObservableAddresses(uint32_t from, uint32_t to);

            Address getAddress() const;

            const api::TezosLikeNetworkParameters getNetworkParameters() const;

            const api::Currency &getCurrency() const;

            Option <std::vector<uint8_t>> getPublicKey() const;

            std::shared_ptr <api::DynamicObject> getConfiguration() const;

            std::string getRestoreKey() const;

            bool contains(const std::string &address) const;

        protected:
            std::shared_ptr <Preferences> getPreferences() const;

        private:
            TezosLikeKeychain::Address getAddressFromPublicKey();

            const api::Currency _currency;
            std::shared_ptr<Preferences> _preferences;
            std::shared_ptr<api::DynamicObject> _configuration;
            std::shared_ptr<TezosLikeAddress> _address;
            Option<std::vector<uint8_t>> _publicKey;
        };
    }
}

