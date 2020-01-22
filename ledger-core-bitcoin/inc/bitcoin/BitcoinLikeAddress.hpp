/*
 *
 * BitcoinLikeAddress
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
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

#include <core/address/Address.hpp>
#include <core/collections/DynamicObject.hpp>
#include <core/utils/Optional.hpp>

#include <bitcoin/api/BitcoinLikeAddress.hpp>
#include <bitcoin/api/BitcoinLikeNetworkParameters.hpp>
#include <bitcoin/api/BitcoinLikeExtendedPublicKey.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeAddress : public api::BitcoinLikeAddress, public Address {
        public:
            BitcoinLikeAddress(const api::Currency& currency,
                               const std::vector<uint8_t>& hash160,
                               const std::string &keychainEngine,
                               const Option<std::string>& derivationPath = Option<std::string>());
            virtual std::vector<uint8_t> getVersion() override;
            std::vector<uint8_t> getVersionFromKeychainEngine(const std::string &keychainEngine,
                                                              const api::BitcoinLikeNetworkParameters &params) const;
            virtual std::vector<uint8_t> getHash160() override;
            virtual api::BitcoinLikeNetworkParameters getNetworkParameters() override;
            virtual std::string toBase58() override;
            virtual std::string toBech32() override;
            std::string toBase58() const;
            std::string toBech32() const;
            virtual bool isP2SH() override;
            virtual bool isP2PKH() override;
            virtual bool isP2WSH() override;
            virtual bool isP2WPKH() override;
            virtual optional<std::string> getDerivationPath() override;

            std::string toString() override;
            std::string getStringAddress() const;

            static std::shared_ptr<Address> parse(const std::string& address, const api::Currency& currency,
                                                          const Option<std::string>& derivationPath = Option<std::string>());
            static std::shared_ptr<BitcoinLikeAddress> fromBase58(const std::string& address,
                                                                  const api::Currency& currency,
                                                                  const Option<std::string>& derivationPath = Option<std::string>());

            static std::shared_ptr<BitcoinLikeAddress> fromBech32(const std::string& address,
                                                                  const api::Currency& currency,
                                                                  const Option<std::string>& derivationPath = Option<std::string>());

            static std::string fromPublicKey(const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &pubKey,
                                             const api::Currency &currency,
                                             const std::string &derivationPath,
                                             const std::string &keychainEngine);

            static std::vector<uint8_t> fromPublicKeyToHash160(const std::vector<uint8_t> &pubKey,
                                                               const std::vector<uint8_t> &pubKeyHash160,
                                                               const api::Currency &currency,
                                                               const std::string &keychainEngine);

            static std::vector<uint8_t> fromPublicKeyToHash160(const std::vector<uint8_t> &pubKey,
                                                               const api::Currency &currency,
                                                               const std::string &keychainEngine);


        private:
            const std::vector<uint8_t> _hash160;
            const api::BitcoinLikeNetworkParameters _params;
            const Option<std::string> _derivationPath;
            const std::string _keychainEngine;
        };
    }
}
