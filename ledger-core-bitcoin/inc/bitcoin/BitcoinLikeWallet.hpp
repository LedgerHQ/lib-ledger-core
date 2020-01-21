/*
 *
 * BitcoinLikeWallet
 * ledger-core
 *
 * Created by Pierre Pollastri on 19/12/2016.
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

#include <memory>

#include <core/Services.hpp>
#include <core/wallet/AbstractWallet.hpp>

#include <bitcoin/api/BitcoinLikeWallet.hpp>
#include <bitcoin/api/BitcoinLikeNetworkParameters.hpp>
#include <bitcoin/database/BitcoinLikeWalletDatabase.hpp>
#include <bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>
#include <bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <bitcoin/observers/BitcoinLikeBlockchainObserver.hpp>
#include <bitcoin/synchronizers/BitcoinLikeAccountSynchronizer.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeWallet : public virtual api::BitcoinLikeWallet, public virtual AbstractWallet {
        public:
            using BitcoinLikeAccountSynchronizerFactory = std::function<std::shared_ptr<BitcoinLikeAccountSynchronizer> ()>;

            BitcoinLikeWallet(
                const std::string& name,
                const std::shared_ptr<BitcoinLikeBlockchainExplorer>& explorer,
                const std::shared_ptr<BitcoinLikeBlockchainObserver>& observer,
                const std::shared_ptr<BitcoinLikeKeychainFactory>& keychainFactory,
                const BitcoinLikeAccountSynchronizerFactory& synchronizerFactory,
                const std::shared_ptr<Services>& services,
                const api::Currency& network,
                const std::shared_ptr<DynamicObject>& configuration,
                const DerivationScheme& scheme
            );

            // API methods
            bool isSynchronizing() override;
            std::shared_ptr<api::EventBus> synchronize() override;

            FuturePtr<ledger::core::api::Account> newAccountWithInfo(const api::AccountCreationInfo &info) override;

            FuturePtr<ledger::core::api::Account>
            newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) override;

            Future<api::ExtendedKeyAccountCreationInfo>
            getExtendedKeyAccountCreationInfo(int32_t accountIndex) override;

            Future<api::AccountCreationInfo> getAccountCreationInfo(int32_t accountIndex) override;

            std::shared_ptr<BitcoinLikeBlockchainExplorer> getBlockchainExplorer();

            bool hasMultipleAddresses() const override;

        protected:
            std::shared_ptr<AbstractAccount>
            createAccountInstance(soci::session &sql, const std::string &accountUid) override;

        private:
            std::shared_ptr<BitcoinLikeWallet> getSelf();

        private:
            std::shared_ptr<BitcoinLikeBlockchainExplorer> _explorer;
            std::shared_ptr<BitcoinLikeBlockchainObserver> _observer;
            std::shared_ptr<BitcoinLikeKeychainFactory> _keychainFactory;
            BitcoinLikeAccountSynchronizerFactory _synchronizerFactory;
        };
    }
}
