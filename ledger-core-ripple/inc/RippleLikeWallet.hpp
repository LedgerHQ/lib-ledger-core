/*
 *
 * RippleLikeWallet
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

#pragma once

#include <core/Services.hpp>
#include <core/wallet/AbstractWallet.hpp>

#include <factories/RippleLikeKeychainFactory.hpp>
#include <factories/RippleLikeWalletFactory.hpp>
#include <explorers/RippleLikeBlockchainExplorer.hpp>
#include <observers/RippleLikeBlockchainObserver.hpp>
#include <synchronizers/RippleLikeAccountSynchronizer.hpp>

namespace ledger {
    namespace core {
        class RippleLikeWallet : public AbstractWallet {
        public:
            RippleLikeWallet(
                    const std::string &name,
                    const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer,
                    const std::shared_ptr<RippleLikeBlockchainObserver> &observer,
                    const RippleLikeAccountSynchronizerFactory &synchronizerFactory,
                    const std::shared_ptr<Services> &services,
                    const api::Currency &network,
                    const std::shared_ptr<DynamicObject> &configuration,
                    const DerivationScheme &scheme
            );

            // API methods
            bool isSynchronizing() override;

            std::shared_ptr<api::EventBus> synchronize() override;

            FuturePtr<api::Account> newAccountWithInfo(const api::AccountCreationInfo &info) override;

            FuturePtr<api::Account>
            newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) override;

            Future<api::ExtendedKeyAccountCreationInfo>
            getExtendedKeyAccountCreationInfo(int32_t accountIndex) override;

            Future<api::AccountCreationInfo> getAccountCreationInfo(int32_t accountIndex) override;

            std::shared_ptr<RippleLikeBlockchainExplorer> getBlockchainExplorer();

            bool hasMultipleAddresses() const override;

        protected:
            std::shared_ptr<AbstractAccount>
            createAccountInstance(soci::session &sql, const std::string &accountUid) override;

        private:
            std::shared_ptr<RippleLikeWallet> getSelf();

            std::shared_ptr<RippleLikeBlockchainExplorer> _explorer;
            std::shared_ptr<RippleLikeBlockchainObserver> _observer;
            std::shared_ptr<RippleLikeKeychainFactory> _keychainFactory;
            RippleLikeAccountSynchronizerFactory _synchronizerFactory;
            api::RippleLikeNetworkParameters _network;
        };
    }
}
