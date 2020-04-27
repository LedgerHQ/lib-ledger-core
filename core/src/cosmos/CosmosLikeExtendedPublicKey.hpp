/*
 *
 * CosmosLikeExtendedPublicKey
 *
 * Created by El Khalil Bellakrid on 13/06/2019.
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


#ifndef LEDGER_CORE_COSMOSLIKEEXTENDEDPUBLICKEY_H
#define LEDGER_CORE_COSMOSLIKEEXTENDEDPUBLICKEY_H

#include <memory>

#include <common/AbstractExtendedPublicKey.h>
#include <crypto/DeterministicPublicKey.hpp>
#include <utils/Option.hpp>
#include <utils/DerivationPath.hpp>
#include <api/Currency.hpp>

#include <api/CosmosLikeExtendedPublicKey.hpp>
#include <api/CosmosLikeNetworkParameters.hpp>
#include <api/CosmosBech32Type.hpp>
#include <api/CosmosLikeAddress.hpp>
#include <api/CosmosCurve.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>

namespace ledger {
    namespace core {
        using CosmosExtendedPublicKey = AbstractExtendedPublicKey<api::CosmosLikeNetworkParameters>;

        class CosmosLikeExtendedPublicKey : public CosmosExtendedPublicKey, public api::CosmosLikeExtendedPublicKey {
        public:

            CosmosLikeExtendedPublicKey(const api::Currency &params,
                                        const DeterministicPublicKey &key,
                                        api::CosmosCurve curve,
                                        api::CosmosBech32Type type,
                                        const DerivationPath &path = DerivationPath("m/"));

            std::shared_ptr<api::CosmosLikeAddress> derive(const std::string &path) override;

            std::shared_ptr<CosmosLikeExtendedPublicKey> derive(const DerivationPath &path);

            std::vector<uint8_t> derivePublicKey(const std::string &path) override;

            std::vector<uint8_t> deriveHash160(const std::string &path) override;

            std::string toBech32() override;

            std::string toBase58() override;

            std::string getRootPath() override;

            static std::shared_ptr<CosmosLikeExtendedPublicKey> fromRaw(const api::Currency &params,
                                                                        const optional<std::vector<uint8_t>> &parentPublicKey,
                                                                        const std::vector<uint8_t> &publicKey,
                                                                        const std::vector<uint8_t> &chainCode,
                                                                        const std::string &path,
                                                                        api::CosmosCurve curve,
                                                                        api::CosmosBech32Type type = api::CosmosBech32Type::PUBLIC_KEY);

            static std::shared_ptr<CosmosLikeExtendedPublicKey> fromBase58(const api::Currency &currency,
                                                                           const std::string &xpub,
                                                                           const Option<std::string> &path,
                                                                           api::CosmosBech32Type type = api::CosmosBech32Type::PUBLIC_KEY_VAL);

            static std::shared_ptr<CosmosLikeExtendedPublicKey> fromBech32(const api::Currency& currency,
                                                                           const std::string& bech32PubKey,
                                                                           const Option<std::string>& path);

        protected:
            virtual const api::CosmosLikeNetworkParameters &params() const override {
                return _currency.cosmosLikeNetworkParameters.value();
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
            api::CosmosCurve _curve;
            api::CosmosBech32Type _type;
        };
    }
}


#endif //LEDGER_CORE_COSMOSLIKEEXTENDEDPUBLICKEY_H
