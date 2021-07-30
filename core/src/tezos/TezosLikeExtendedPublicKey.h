/*
 *
 * TezosLikeExtendedPublicKey
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
#ifndef LEDGER_CORE_TEZOSLIKEEXTENDEDPUBLICKEY_H
#define LEDGER_CORE_TEZOSLIKEEXTENDEDPUBLICKEY_H

#include <common/AbstractExtendedPublicKey.h>
#include <api/TezosLikeExtendedPublicKey.hpp>
#include <crypto/DeterministicPublicKey.hpp>
#include <api/TezosLikeNetworkParameters.hpp>
#include <memory>
#include <utils/Option.hpp>
#include <utils/DerivationPath.hpp>
#include <api/Currency.hpp>
#include <api/TezosCurve.hpp>
#include <tezos/TezosKey.h>

namespace ledger {
    namespace core {
        using TezosExtendedPublicKey = AbstractExtendedPublicKey<api::TezosLikeNetworkParameters>;

        class TezosLikeExtendedPublicKey : public TezosExtendedPublicKey, public api::TezosLikeExtendedPublicKey {
        public:

            TezosLikeExtendedPublicKey(const api::Currency &params,
                                       const DeterministicPublicKey &key,
                                       api::TezosCurve curve,
                                       const DerivationPath &path = DerivationPath("m/"));

            std::shared_ptr<api::TezosLikeAddress> derive(const std::string &path) override;

            std::shared_ptr<TezosLikeExtendedPublicKey> derive(const DerivationPath &path);

            std::vector<uint8_t> derivePublicKey(const std::string &path) override;

            std::vector<uint8_t> deriveHash160(const std::string &path) override;

            std::string toBase58() override;

            std::string getRootPath() override;

            static std::shared_ptr<TezosLikeExtendedPublicKey> fromRaw(const api::Currency &params,
                                                                       const optional<std::vector<uint8_t>> &parentPublicKey,
                                                                       const std::vector<uint8_t> &publicKey,
                                                                       const std::vector<uint8_t> &chainCode,
                                                                       const std::string &path,
                                                                       api::TezosCurve curve);

            static std::shared_ptr<TezosLikeExtendedPublicKey> fromBase58(const api::Currency &currency,
                                                                          const std::string &xpubBase58,
                                                                          const Option<std::string> &path);


            static const api::TezosCurve getCurveFromPrefix(std::string prefix);

        protected:
            const api::TezosLikeNetworkParameters &params() const override {
                return _currency.tezosLikeNetworkParameters.value();
            };

            const DeterministicPublicKey &getKey() const override {
                return _key;
            };

            const DerivationPath &getPath() const override {
                return _path;
            };

            const api::Currency &getCurrency() const override {
                return _currency;
            };
        private:
            const api::Currency _currency;
            const DerivationPath _path;
            const DeterministicPublicKey _key;
            api::TezosCurve _curve;
            TezosKeyType::Encoding  _encoding;
        };
    }
}


#endif //LEDGER_CORE_TEZOSLIKEEXTENDEDPUBLICKEY_H
