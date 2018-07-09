/*
 *
 * EthereumLikeExtendedPublicKey
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
#ifndef LEDGER_CORE_ETHEREUMLIKEEXTENDEDPUBLICKEY_H
#define LEDGER_CORE_ETHEREUMLIKEEXTENDEDPUBLICKEY_H

#include <api/EthereumLikeExtendedPublicKey.hpp>
#include <crypto/DeterministicPublicKey.hpp>
#include <api/EthereumLikeNetworkParameters.hpp>
#include <memory>
#include <utils/Option.hpp>
#include <utils/DerivationPath.hpp>
#include <api/Currency.hpp>

namespace ledger {
    namespace core {
        class EthereumLikeExtendedPublicKey : public api::EthereumLikeExtendedPublicKey {
        public:

            EthereumLikeExtendedPublicKey(const api::Currency& params,
                                          const DeterministicPublicKey& key,
                                          const DerivationPath& path = DerivationPath("m/"));

            std::shared_ptr<api::EthereumLikeAddress> derive(const std::string & path) override ;
            std::shared_ptr<EthereumLikeExtendedPublicKey> derive(const DerivationPath &path);
            std::vector<uint8_t> derivePublicKey(const std::string & path) override ;

            std::vector<uint8_t> deriveHash160(const std::string & path) override ;

            std::string toBase58() override ;

            std::string getRootPath() override ;

            static std::shared_ptr<EthereumLikeExtendedPublicKey> fromBase58(
                    const api::Currency& currency,
                    const std::string& xpubBase58,
                    const Option<std::string>& path
            );

        private:
            const api::Currency _currency;
            const DerivationPath _path;
            const DeterministicPublicKey _key;

        };
    }
}


#endif //LEDGER_CORE_ETHEREUMLIKEEXTENDEDPUBLICKEY_H
