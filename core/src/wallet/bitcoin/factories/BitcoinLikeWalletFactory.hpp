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

#include <wallet/common/AbstractWalletFactory.hpp>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>
#include <wallet/bitcoin/synchronizers/BitcoinLikeAccountSynchronizer.hpp>
#include <wallet/bitcoin/observers/BitcoinLikeBlockchainObserver.hpp>
#include <wallet/bitcoin/synchronizers/BitcoinLikeAccountSynchronizer.hpp>
#include "BitcoinLikeKeychainFactory.h"

namespace ledger {
    namespace core {

        using BitcoinLikeAccountSynchronizerFactory = std::function<std::shared_ptr<BitcoinLikeAccountSynchronizer> ()>;
        class WalletPool;

        class BitcoinLikeWalletFactory : public AbstractWalletFactory {
        public:
            BitcoinLikeWalletFactory(const api::Currency &currency, const std::shared_ptr<WalletPool> &pool);
            std::shared_ptr<AbstractWallet> build(const WalletDatabaseEntry &entry) override;

        private:
            std::shared_ptr<BitcoinLikeBlockchainExplorer> getExplorer(const std::string& currencyName, const std::shared_ptr<api::DynamicObject>& configuration);
            std::shared_ptr<BitcoinLikeBlockchainObserver> getObserver(const std::string& currencyName, const std::shared_ptr<api::DynamicObject>& configuration);
        private:
            // Explorers
            std::list<std::weak_ptr<BitcoinLikeBlockchainExplorer>> _runningExplorers;

            // Observers
            std::list<std::weak_ptr<BitcoinLikeBlockchainObserver>> _runningObservers;

            // Keychain factories
            std::unordered_map<std::string, std::shared_ptr<BitcoinLikeKeychainFactory>> _keychainFactories;

        };
    }
}

#endif //LEDGER_CORE_BITCOINLIKEWALLETFACTORY_HPP
