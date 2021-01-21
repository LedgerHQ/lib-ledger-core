/*
 *
 * RippleLikeWalletFactory
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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


#ifndef LEDGER_CORE_RIPPLELIKEWALLETFACTORY_H
#define LEDGER_CORE_RIPPLELIKEWALLETFACTORY_H


#include <functional>

#include <wallet/common/AbstractWalletFactory.hpp>
#include <wallet/ripple/synchronizers/RippleLikeAccountSynchronizer.h>
#include <wallet/ripple/explorers/RippleLikeBlockchainExplorer.h>
#include <wallet/ripple/factories/RippleLikeKeychainFactory.h>

#include <api/Currency.hpp>

namespace ledger {
    namespace core {

        using RippleLikeAccountSynchronizerFactory = std::function<std::shared_ptr<RippleLikeAccountSynchronizer> ()>;
        class WalletPool;

        class RippleLikeWalletFactory : public AbstractWalletFactory {
        public:
            RippleLikeWalletFactory(const api::Currency &currency, const std::shared_ptr<WalletPool> &pool);
            std::shared_ptr<AbstractWallet> build(const WalletDatabaseEntry &entry) override;

        private:
            std::shared_ptr<RippleLikeBlockchainExplorer> getExplorer(const std::string& currencyName, const std::shared_ptr<api::DynamicObject>& configuration);
        private:
            // Explorers
            std::list<std::weak_ptr<RippleLikeBlockchainExplorer>> _runningExplorers;

            // Keychain factories
            std::unordered_map<std::string, std::shared_ptr<RippleLikeKeychainFactory>> _keychainFactories;

        };

    }
}


#endif //LEDGER_CORE_RIPPLELIKEWALLETFACTORY_H
