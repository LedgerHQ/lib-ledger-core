/*
 *
 * StellarLikeAddress.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
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

#ifndef LEDGER_CORE_STELLARLIKEADDRESS_HPP
#define LEDGER_CORE_STELLARLIKEADDRESS_HPP

#include <wallet/common/AbstractAddress.h>
#include <api/StellarLikeAddress.hpp>
#include <wallet/stellar/xdr/models.hpp>

namespace ledger {
    namespace core {
        class StellarLikeAddress : public virtual api::StellarLikeAddress, public virtual AbstractAddress {
        public:
            StellarLikeAddress( const std::vector<uint8_t>& pubKey,
                                const api::Currency& currency,
                                const Option<std::string>& path);
            StellarLikeAddress( const std::string& address,
                                const api::Currency& currency,
                                const Option<std::string>& path);
            std::string toString() override;
            static std::shared_ptr<StellarLikeAddress> parse(const std::string& address, const api::Currency& currency);

            static bool isValid(const std::string& address, const api::Currency& currency);

            static std::string convertPubkeyToAddress(  const std::vector<uint8_t>& pubKey,
                                                        const api::StellarLikeNetworkParameters& params);

            static std::string convertXdrAccountToAddress(const stellar::xdr::AccountID& accountId,
                                                          const api::StellarLikeNetworkParameters& params);
            std::vector<uint8_t> toPublicKey() const;
            stellar::xdr::PublicKey toXdrPublicKey() const;

        private:
            std::string _address;
        };
    }
}


#endif //LEDGER_CORE_STELLARLIKEADDRESS_HPP
