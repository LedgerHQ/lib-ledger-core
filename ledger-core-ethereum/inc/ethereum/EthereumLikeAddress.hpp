/*
 *
 * EthereumLikeAddress
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

#pragma once

#include <core/address/Address.hpp>
#include <core/utils/Optional.hpp>
#include <core/api/Currency.hpp>

#include <ethereum/api/EthereumLikeNetworkParameters.hpp>
#include <ethereum/api/EthereumLikeAddress.hpp>

namespace ledger {
    namespace core {
        class EthereumLikeAddress : public api::EthereumLikeAddress, public Address {
        public:
            EthereumLikeAddress(const api::Currency& currency,
                               const std::vector<uint8_t>& keccak256,
                               const Option<std::string>& derivationPath = Option<std::string>());
            std::vector<uint8_t> getVersion() override ;
            std::vector<uint8_t> getKeccakHash() override ;
            api::EthereumLikeNetworkParameters getNetworkParameters() override ;
            std::string toEIP55() override ;

            virtual optional<std::string> getDerivationPath() override;
            std::string toString() override;

            // Attempt to parse a string address.
            // @param address The string to parse
            // @param currency The currency used to parse the address
            // @return If successful, returns the address, otherwise none.
            static std::shared_ptr<Address> parse(
                const std::string& address, const api::Currency& currency,
                const Option<std::string>& derivationPath = Option<std::string>());

            // Check whether the given string formatted address is valid or not.
            // @param address The string to parse
            // @param currency The currency used to parse the address
            // @return If successful, returns true; false otherwise.
            static bool isValid(
                const std::string &address,
                const api::Currency &currency,
                const Option<std::string>& derivationPath = Option<std::string>());

            static std::shared_ptr<EthereumLikeAddress> fromEIP55(const std::string& address,
                                                                  const api::Currency& currency,
                                                                  const Option<std::string>& derivationPath = Option<std::string>(),
                                                                  bool skipEIP55Check = false);
        private:
            const std::vector<uint8_t> _keccak256;
            const api::EthereumLikeNetworkParameters _params;
            const Option<std::string> _derivationPath;
        };
    }
}