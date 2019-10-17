/*
 *
 * RippleLikeExtendedPublicKey
 *
 * Created by El Khalil Bellakrid on 05/01/2019.
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

#pragma once

#include <memory>

#include <core/api/Currency.hpp>
#include <core/crypto/DeterministicPublicKey.hpp>
#include <core/key/ExtendedPublicKey.hpp>
#include <core/utils/DerivationPath.hpp>
#include <core/utils/Option.hpp>

#include <ripple/api/RippleLikeExtendedPublicKey.hpp>
#include <ripple/api/RippleLikeNetworkParameters.hpp>
#include <ripple/RippleLikeAddress.hpp>

namespace ledger {
    namespace core {
        using RippleExtendedPublicKey = ExtendedPublicKey<api::RippleLikeNetworkParameters>;

        class RippleLikeExtendedPublicKey : public RippleExtendedPublicKey, public api::RippleLikeExtendedPublicKey {
        public:
            RippleLikeExtendedPublicKey(
                const api::Currency& params,
                const DeterministicPublicKey& key,
                const DerivationPath& path = DerivationPath("m/")
            );

            std::shared_ptr<api::RippleLikeAddress> derive(const std::string & path) override ;
            std::shared_ptr<RippleLikeExtendedPublicKey> derive(const DerivationPath &path);
            std::vector<uint8_t> derivePublicKey(const std::string & path) override ;

            std::vector<uint8_t> deriveHash160(const std::string & path) override ;

            std::string toBase58() override ;

            std::string getRootPath() override ;

            static std::shared_ptr<RippleLikeExtendedPublicKey> fromRaw(
                const api::Currency& params,
                const optional<std::vector<uint8_t>>& parentPublicKey,
                const std::vector<uint8_t>& publicKey,
                const std::vector<uint8_t> &chainCode,
                const std::string& path
            );

            static std::shared_ptr<RippleLikeExtendedPublicKey> fromBase58(
                const api::Currency& currency,
                const std::string& xpubBase58,
                const Option<std::string>& path
            );

        protected:
            const api::RippleLikeNetworkParameters params() const override;

            const DeterministicPublicKey& getKey() const override;

            const DerivationPath& getPath() const override;

            const api::Currency& getCurrency() const override;

        private:
            const api::Currency _currency;
            const DerivationPath _path;
            const DeterministicPublicKey _key;
        };
    }
}
