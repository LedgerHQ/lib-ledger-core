/*
 * AlgorandWallet
 *
 * Created by Hakim Aammar on 20/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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


#ifndef LEDGER_CORE_ALGORANDWALLET_H
#define LEDGER_CORE_ALGORANDWALLET_H

#include <algorand/AlgorandAccountSynchronizer.hpp>
#include <algorand/AlgorandBlockchainExplorer.hpp>
#include <algorand/AlgorandBlockchainObserver.hpp>

#include <algorand/api/AlgorandWallet.hpp>

#include <core/wallet/AbstractWallet.hpp>

namespace ledger {
namespace core {
namespace algorand {

    // FIXME Temporary, to be removed or put elsewhere
    using AccountSynchronizerFactory = std::function<std::shared_ptr<AccountSynchronizer>()>;

    class Wallet : public api::AlgorandWallet, public AbstractWallet {

    public:

        Wallet(
            const std::string &name,
            const api::Currency &currency,
            const std::shared_ptr<Services> &services,
            const std::shared_ptr<DynamicObject> &configuration,
            const DerivationScheme &scheme,
            const std::shared_ptr<BlockchainExplorer> &explorer, // TODO Could be a singleton service?
            const std::shared_ptr<BlockchainObserver> &observer,
            const AccountSynchronizerFactory &synchronizerFactory
        );

        bool isSynchronizing() override;

        std::shared_ptr<api::EventBus> synchronize() override;

        FuturePtr<ledger::core::api::Account> newAccountWithInfo(const api::AccountCreationInfo &info) override;

        FuturePtr<ledger::core::api::Account>
        newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) override;

        Future<api::ExtendedKeyAccountCreationInfo>
        getExtendedKeyAccountCreationInfo(int32_t accountIndex) override;

        Future<api::AccountCreationInfo> getAccountCreationInfo(int32_t accountIndex) override;

        bool hasMultipleAddresses() const override;

    protected:
        std::shared_ptr<AbstractAccount>
        createAccountInstance(soci::session &sql, const std::string &accountUid) override;

    private:
        std::shared_ptr<Wallet> getSelf();

        std::shared_ptr<BlockchainExplorer> _explorer;
        std::shared_ptr<BlockchainObserver> _observer;
        AccountSynchronizerFactory _synchronizerFactory;
    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDWALLET_H
