/*
 *
 * DefaultStellarLikeKeychain.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/02/2019.
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

#ifndef LEDGER_CORE_DEFAULTSTELLARLIKEKEYCHAIN_HPP
#define LEDGER_CORE_DEFAULTSTELLARLIKEKEYCHAIN_HPP

#include "StellarLikeKeychain.hpp"

namespace ledger {
    namespace core {
        class DefaultStellarLikeKeychain : public StellarLikeKeychain {
        public:
            DefaultStellarLikeKeychain( const Address& address,
                                        const std::shared_ptr<api::DynamicObject>& configuration,
                                        const api::Currency& currency,
                                        const std::shared_ptr<Preferences>& preferences);

            Address getAddress() const override;

            bool contains(const std::string &address) const override;

            std::string getRestoreKey() const override;

        private:
            Address _address;
            std::vector<uint8_t> _pubKey;
        };
    }
}


#endif //LEDGER_CORE_DEFAULTSTELLARLIKEKEYCHAIN_HPP
