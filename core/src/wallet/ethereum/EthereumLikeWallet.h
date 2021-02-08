/*
 *
 * EthereumLikeWallet
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#ifndef LEDGER_CORE_ETHEREUMLIKEWALLET_H
#define LEDGER_CORE_ETHEREUMLIKEWALLET_H

#include <wallet/common/AbstractWallet.hpp>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>
#include <wallet/ethereum/synchronizers/EthereumLikeAccountSynchronizer.h>
#include <wallet/ethereum/factories/EthereumLikeWalletFactory.h>
#include <wallet/ethereum/factories/EthereumLikeKeychainFactory.h>
namespace ledger {
    namespace core {
        class EthereumLikeWallet : public AbstractWallet {
        public:
            static const api::WalletType type;

            EthereumLikeWallet(
                    const std::string& name,
                    const std::shared_ptr<EthereumLikeBlockchainExplorer>& explorer,
                    const std::shared_ptr<EthereumLikeKeychainFactory>& keychainFactory,
                    const EthereumLikeAccountSynchronizerFactory& synchronizerFactory,
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

            std::shared_ptr<EthereumLikeBlockchainExplorer> getBlockchainExplorer();

        protected:
            std::shared_ptr<AbstractAccount>
            createAccountInstance(soci::session &sql, const std::string &accountUid) override;

        private:
            std::shared_ptr<EthereumLikeWallet> getSelf();

            std::shared_ptr<EthereumLikeBlockchainExplorer> _explorer;
            std::shared_ptr<EthereumLikeKeychainFactory> _keychainFactory;
            EthereumLikeAccountSynchronizerFactory _synchronizerFactory;
            api::EthereumLikeNetworkParameters _network;
            int _coinType;
        };        
    }
}



#endif //LEDGER_CORE_ETHEREUMLIKEWALLET_H
