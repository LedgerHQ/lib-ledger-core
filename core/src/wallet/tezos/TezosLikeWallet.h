/*
 *
 * TezosLikeWallet
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#ifndef LEDGER_CORE_TEZOSLIKEWALLET_H
#define LEDGER_CORE_TEZOSLIKEWALLET_H
#include <wallet/common/AbstractWallet.hpp>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/tezos/synchronizers/TezosLikeAccountSynchronizer.hpp>
#include <wallet/tezos/factories/TezosLikeWalletFactory.h>
#include <wallet/tezos/factories/TezosLikeKeychainFactory.h>

namespace ledger {
    namespace core {
        class TezosLikeWallet : public AbstractWallet {
        public:
            static const api::WalletType type;

            TezosLikeWallet(
                    const std::string &name,
                    const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                    const std::shared_ptr<TezosLikeKeychainFactory> &keychainFactory,
                    const TezosLikeAccountSynchronizerFactory &synchronizerFactory,
                    const std::shared_ptr<WalletPool> &pool,
                    const api::Currency &network,
                    const std::shared_ptr<DynamicObject> &configuration,
                    const DerivationScheme &scheme
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

            std::shared_ptr<TezosLikeBlockchainExplorer> getBlockchainExplorer();

        protected:
            std::shared_ptr<AbstractAccount>
            createAccountInstance(soci::session &sql, const std::string &accountUid) override;

        private:
            std::shared_ptr<TezosLikeWallet> getSelf();

            std::shared_ptr<TezosLikeBlockchainExplorer> _explorer;
            std::shared_ptr<TezosLikeKeychainFactory> _keychainFactory;
            TezosLikeAccountSynchronizerFactory _synchronizerFactory;
            api::TezosLikeNetworkParameters _network;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEWALLET_H
