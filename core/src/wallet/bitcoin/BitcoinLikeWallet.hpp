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

#include <api/BitcoinLikeNetworkParameters.hpp>
#include <api/BitcoinLikeWallet.hpp>
#include <memory>
#include <wallet/AccountSynchronizerFactory.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/TransactionBroadcaster.hpp>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/bitcoin/factories/AccountSynchronizerFactory.hpp>
#include <wallet/bitcoin/factories/BitcoinLikeKeychainFactory.h>
#include <wallet/bitcoin/factories/BitcoinLikeWalletFactory.hpp>
#include "wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp"
#include "wallet/bitcoin/observers/BitcoinLikeBlockchainObserver.hpp"
#include "wallet/bitcoin/synchronizers/AccountSynchronizer.hpp"
#include <wallet/common/AbstractWallet.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class BitcoinLikeWallet : public virtual api::BitcoinLikeWallet, public virtual AbstractWallet {
            public:
                static const api::WalletType type;
                typedef typename TransactionBroadcaster<BitcoinLikeNetwork> TransactionBroadcaster;
                
                BitcoinLikeWallet(
                    const std::string& name,
                    const std::shared_ptr<TransactionBroadcaster>& transactionBroadcaster,
                    const std::shared_ptr<BitcoinLikeBlockchainObserver>& observer,
                    const std::shared_ptr<BitcoinLikeKeychainFactory>& keychainFactory,
                    const std::shared_ptr<AccountSynchronizerFactory>& synchronizerFactory,
                    const std::shared_ptr<WalletPool>& pool,
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

            protected:
                std::shared_ptr<AbstractAccount>
                    createAccountInstance(soci::session &sql, const std::string &accountUid) override;

            private:
                std::shared_ptr<BitcoinLikeWallet> getSelf();

            private:
                std::shared_ptr<TransactionBroadcaster> _transactionBroadcaster;
                std::shared_ptr<BitcoinLikeBlockchainObserver> _observer;
                std::shared_ptr<BitcoinLikeKeychainFactory> _keychainFactory;
                std::shared_ptr<AccountSynchronizerFactory> _synchronizerFactory;
                api::BitcoinLikeNetworkParameters _network;
            };
        }
    }
}
