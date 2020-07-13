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

#pragma once 

#include <string>
#include <vector>

#include <core/api/Configuration.hpp>
#include <core/api/Currency.hpp>
#include <core/api/AccountCreationInfo.hpp>
#include <core/api/ExtendedKeyAccountCreationInfo.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/preferences/Preferences.hpp>
#include <core/utils/DerivationScheme.hpp>
#include <core/utils/Option.hpp>

#include <bitcoin/BitcoinLikeAddress.hpp>
#include <bitcoin/BitcoinLikeExtendedPublicKey.hpp>

namespace ledger {
    namespace core {

        class BitcoinLikeKeychain {
        public:
            enum KeyPurpose {
                RECEIVE, CHANGE
            };

        public:
            using Address = std::shared_ptr<BitcoinLikeAddress>;

            BitcoinLikeKeychain(
                    const std::shared_ptr<api::DynamicObject>& configuration,
                    const api::Currency& params,
                    int account,
                    const std::shared_ptr<Preferences>& preferences);

            virtual bool markAsUsed(const std::vector<std::string>& addresses);
            virtual bool markAsUsed(const std::string& address);
            virtual bool markPathAsUsed(const DerivationPath& path) = 0;

            virtual std::vector<Address> getAllObservableAddresses(uint32_t from, uint32_t to)  = 0;
            virtual std::vector<Address> getAllObservableAddresses(KeyPurpose purpose, uint32_t from, uint32_t to) = 0;

            virtual Address getFreshAddress(KeyPurpose purpose) = 0;
            virtual std::vector<Address> getFreshAddresses(KeyPurpose purpose, size_t n) = 0;

            virtual Option<KeyPurpose> getAddressPurpose(const std::string& address) const = 0;
            virtual Option<std::string> getAddressDerivationPath(const std::string& address) const = 0;
            virtual bool isEmpty() const = 0;

            int getAccountIndex() const;
            const api::BitcoinLikeNetworkParameters getNetworkParameters() const;
            const api::Currency& getCurrency() const;

            virtual Option<std::vector<uint8_t>> getPublicKey(const std::string& address) const = 0;


            std::shared_ptr<api::DynamicObject> getConfiguration() const;
            const DerivationScheme& getDerivationScheme() const;
            const DerivationScheme& getFullDerivationScheme() const;
            std::string getKeychainEngine() const;
            bool isSegwit() const;
            bool isNativeSegwit() const;

            virtual std::string getRestoreKey() const = 0;
            virtual int32_t getObservableRangeSize() const = 0;
            virtual bool contains(const std::string& address) const = 0;
            virtual std::vector<Address> getAllAddresses() = 0;
            virtual int32_t getOutputSizeAsSignedTxInput() const = 0;

            static bool isSegwit(const std::string &keychainEngine);
            static bool isNativeSegwit(const std::string &keychainEngine);
        protected:
            std::shared_ptr<Preferences> getPreferences() const;
            DerivationScheme& getDerivationScheme();

        private:
            const api::Currency _currency;
            DerivationScheme _scheme;
            DerivationScheme _fullScheme;
            int _account;
            std::shared_ptr<Preferences> _preferences;
            std::shared_ptr<api::DynamicObject> _configuration;
        };
    }
}
