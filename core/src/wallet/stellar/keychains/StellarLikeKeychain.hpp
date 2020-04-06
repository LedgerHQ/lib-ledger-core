/*
 *
 * StellarLikeKeychain.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/02/2019.
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

#ifndef LEDGER_CORE_STELLARLIKEKEYCHAIN_HPP
#define LEDGER_CORE_STELLARLIKEKEYCHAIN_HPP

#include <wallet/stellar/StellarLikeAddress.hpp>
#include <collections/DynamicObject.hpp>
#include <preferences/Preferences.hpp>

namespace ledger {
    namespace core {
        class StellarLikeKeychain {
        public:
            using Address = std::shared_ptr<StellarLikeAddress>;

            StellarLikeKeychain(const std::shared_ptr<api::DynamicObject>& configuration,
                                const api::Currency& currency,
                                const std::shared_ptr<Preferences>& preferences);
            const api::StellarLikeNetworkParameters& getNetworkParams() const;
            const api::Currency& getCurrency() const;
            virtual Address getAddress() const = 0;
            virtual bool contains(const std::string& address) const = 0;
            virtual std::string getRestoreKey() const = 0;

        protected:
            const std::shared_ptr<api::DynamicObject>& getConfiguration() const;
            const std::shared_ptr<Preferences>& getPreferences() const;

        private:
            std::shared_ptr<api::DynamicObject> _configuration;
            std::shared_ptr<Preferences> _preferences;
            api::Currency _currency;
        };
    }
}


#endif //LEDGER_CORE_STELLARLIKEKEYCHAIN_HPP
