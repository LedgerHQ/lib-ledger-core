/*
 *
 * BitcoinLikeWalletFactory
 * ledger-core
 *
 * Created by Pierre Pollastri on 31/01/2017.
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
#ifndef LEDGER_CORE_BITCOINLIKEWALLETFACTORY_HPP
#define LEDGER_CORE_BITCOINLIKEWALLETFACTORY_HPP

#include "../BitcoinLikeWallet.hpp"
#include "../../../api/BitcoinLikeExtendedPublicKeyProvider.hpp"
#include "../../../api/BitcoinLikeNetworkParameters.hpp"
#include "../../../api/Configuration.hpp"
#include "../../../preferences/Preferences.hpp"
#include "../../../utils/Option.hpp"
#include "../../pool/WalletPool.hpp"

#include <memory>
#include <vector>

namespace ledger {
    namespace core {
        struct BitcoinLikeWalletEntry : public WalletPool::WalletEntry {

            template <typename Archive>
            void serialize(Archive& ar) {
                ar(cereal::base_class<WalletPool::WalletEntry>(this));
            };
        };

        class BitcoinLikeWalletFactory {
        public:
            BitcoinLikeWalletFactory(const api::BitcoinLikeNetworkParameters& params,
                                     std::shared_ptr<WalletPool> pool,
                                     std::shared_ptr<Preferences> preferences);
            Future<std::shared_ptr<BitcoinLikeWallet>> build(const std::string identifier);
            Future<std::shared_ptr<BitcoinLikeWallet>> build(
                    std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> provider,
                    std::shared_ptr<api::DynamicObject> configuration
            );

            static void initialize(const std::shared_ptr<Preferences>& preferences, Map<std::string, api::BitcoinLikeNetworkParameters>& networks);
        private:
            Future<std::shared_ptr<BitcoinLikeWallet>> build(const BitcoinLikeWalletEntry& entry);

        private:
            std::shared_ptr<Preferences> _preferences;
            api::BitcoinLikeNetworkParameters _params;
            std::shared_ptr<WalletPool> _pool;
        };
    }
}

CEREAL_REGISTER_TYPE(ledger::core::BitcoinLikeWalletEntry);
CEREAL_REGISTER_POLYMORPHIC_RELATION(ledger::core::WalletPool::WalletEntry, ledger::core::BitcoinLikeWalletEntry);

#endif //LEDGER_CORE_BITCOINLIKEWALLETFACTORY_HPP
