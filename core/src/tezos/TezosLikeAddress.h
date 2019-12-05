/*
 *
 * TezosLikeAddress
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 25/04/2019.
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
#ifndef LEDGER_CORE_TEZOSLIKEADDRESS_H
#define LEDGER_CORE_TEZOSLIKEADDRESS_H

#include <api/TezosLikeAddress.hpp>
#include <api/TezosLikeNetworkParameters.hpp>
#include "../utils/optional.hpp"
#include <wallet/common/AbstractAddress.h>
#include <api/TezosCurve.hpp>

namespace ledger {
    namespace core {
        class TezosLikeAddress : public api::TezosLikeAddress, public AbstractAddress {
        public:
            TezosLikeAddress(const api::Currency &currency,
                             const std::vector<uint8_t> &hash160,
                             const std::vector<uint8_t> &version,
                             const Option<std::string> &derivationPath = Option<std::string>());

            std::vector<uint8_t> getVersion() override;

            std::vector<uint8_t> getHash160() override;

            api::TezosLikeNetworkParameters getNetworkParameters() override;

            std::string toBase58() override;

            virtual optional<std::string> getDerivationPath() override;

            std::string toString() override;

            static std::shared_ptr<AbstractAddress> parse(const std::string &address,
                                                          const api::Currency &currency,
                                                          const Option<std::string> &derivationPath = Option<std::string>());

            static std::shared_ptr<TezosLikeAddress> fromBase58(const std::string &address,
                                                                const api::Currency &currency,
                                                                const Option<std::string> &derivationPath = Option<std::string>());

        private:
            const std::vector<uint8_t> _version;
            const std::vector<uint8_t> _hash160;
            const api::TezosLikeNetworkParameters _params;
            const Option<std::string> _derivationPath;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEADDRESS_H
