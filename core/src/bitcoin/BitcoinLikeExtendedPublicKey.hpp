/*
 *
 * BitcoinLikeExtendedPublicKey
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/12/2016.
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
#ifndef LEDGER_CORE_BITCOINLIKEEXTENDEDPUBLICKEY_HPP
#define LEDGER_CORE_BITCOINLIKEEXTENDEDPUBLICKEY_HPP

#include <api/BitcoinLikeExtendedPublicKey.hpp>
#include <api/BitcoinLikeNetworkParameters.hpp>
#include <api/Currency.hpp>
#include <common/AbstractExtendedPublicKey.h>
#include <crypto/DeterministicPublicKey.hpp>
#include <memory>
#include <utils/DerivationPath.hpp>
#include <utils/Option.hpp>

namespace ledger {
    namespace core {
        using BitcoinExtendedPublicKey = AbstractExtendedPublicKey<api::BitcoinLikeNetworkParameters>;
        class BitcoinLikeExtendedPublicKey : public BitcoinExtendedPublicKey, public api::BitcoinLikeExtendedPublicKey {
          public:
            BitcoinLikeExtendedPublicKey(const api::Currency &params,
                                         const DeterministicPublicKey &key,
                                         const std::shared_ptr<DynamicObject> &configuration,
                                         const DerivationPath &path = DerivationPath("m/"));
            std::shared_ptr<api::BitcoinLikeAddress> derive(const std::string &path) override;
            std::shared_ptr<BitcoinLikeExtendedPublicKey> derive(const DerivationPath &path);

            std::vector<uint8_t> derivePublicKey(const std::string &path) override;

            std::vector<uint8_t> deriveHash160(const std::string &path) override;

            std::string toBase58() override;

            std::string getRootPath() override;

          public:
            static std::shared_ptr<BitcoinLikeExtendedPublicKey> fromRaw(
                const api::Currency &params,
                const optional<std::vector<uint8_t>> &parentPublicKey,
                const std::vector<uint8_t> &publicKey,
                const std::vector<uint8_t> &chainCode,
                const std::string &path,
                const std::shared_ptr<DynamicObject> &configuration);

            static std::shared_ptr<BitcoinLikeExtendedPublicKey> fromBase58(
                const api::Currency &currency,
                const std::string &xpubBase58,
                const Option<std::string> &path,
                const std::shared_ptr<DynamicObject> &configuration);

          protected:
            const api::BitcoinLikeNetworkParameters &params() const override {
                return _currency.bitcoinLikeNetworkParameters.value();
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
            const std::shared_ptr<DynamicObject> _configuration;
        };
    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_BITCOINLIKEEXTENDEDPUBLICKEY_HPP
