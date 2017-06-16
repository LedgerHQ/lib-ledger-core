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
#include "BitcoinLikeWallet.hpp"
#include <api/ErrorCode.hpp>
#include <api/AccountCallback.hpp>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <api/BitcoinLikeExtendedPublicKeyProvider.hpp>
#include <bitcoin/BitcoinLikeExtendedPublicKeyProvider.hpp>
#include <api/ConfigurationDefaults.hpp>
#include <api/KeychainEngines.hpp>
#include <wallet/bitcoin/database/BitcoinLikeAccountDatabaseHelper.h>
#include "BitcoinLikeAccount.hpp"

namespace ledger {
    namespace core {

        const api::WalletType BitcoinLikeWallet::type = api::WalletType::BITCOIN;

        BitcoinLikeWallet::BitcoinLikeWallet(const std::string &name,
                                             const std::shared_ptr<BitcoinLikeBlockchainObserver> &observer,
                                             const std::shared_ptr<BitcoinLikeKeychainFactory> &keychainFactory,
                                             const BitcoinLikeAccountSynchronizerFactory &synchronizer,
                                             const std::shared_ptr<WalletPool> &pool, const api::Currency &network,
                                             const std::shared_ptr<DynamicObject>& configuration,
                                             const DerivationScheme& scheme
        )
        : AbstractWallet(name, network, pool, configuration, scheme) {
            _observer = observer;
            _keychainFactory = keychainFactory;
            _synchronizerFactory = synchronizer;
        }

        void BitcoinLikeWallet::getAccount(int32_t index, const std::shared_ptr<api::AccountCallback> &callback) {

        }

        void BitcoinLikeWallet::getAccountCount(const std::shared_ptr<api::I32Callback> &callback) {

        }

        void BitcoinLikeWallet::getAccounts(int32_t offset, int32_t count,
                                            const std::shared_ptr<api::AccountListCallback> &callback) {

        }

        bool BitcoinLikeWallet::isSynchronizing() {
            return false;
        }

        std::shared_ptr<api::EventBus> BitcoinLikeWallet::synchronize() {
            return nullptr;
        }

        void BitcoinLikeWallet::createNewAccount(int32_t index,
                                                 const std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> &xpubProvider,
                                                 const std::shared_ptr<api::AccountCallback> &callback) {
            createNewAccount(index, xpubProvider).callback(getMainExecutionContext(), callback);
        }

        void BitcoinLikeWallet::createNextAccount(
                const std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> &xpubProvider,
                const std::shared_ptr<api::AccountCallback> &callback) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeWallet>(shared_from_this());
            getNextAccountIndex().flatMapPtr<ledger::core::api::Account>(getContext(), [=] (const int32_t& index) {
                return self->createNewAccount(index, xpubProvider);
            });
        }

        FuturePtr<ledger::core::api::Account> BitcoinLikeWallet::createNewAccount(int32_t index,
                                                                                  const std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> &xpubProvider) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeWallet>(shared_from_this());
            auto provider = std::dynamic_pointer_cast<BitcoinLikeExtendedPublicKeyProvider>(xpubProvider);
            auto scheme = getDerivationScheme();
            scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(index);

            return _keychainFactory->build(getContext(),
                                           index,
                                    scheme.getPath(),
                                    getConfiguration(),
                                    provider,
                                    getAccountInternalPreferences(index),
                                    getCurrency()).map<std::shared_ptr<api::Account>>(getContext(), [=] (const std::shared_ptr<BitcoinLikeKeychain>& keychain) {
                soci::session sql(self->getDatabase()->getPool());
                sql.begin();
                auto accountUid = AccountDatabaseHelper::createAccountUid(self->getWalletUid(), index);
                AccountDatabaseHelper::createAccount(sql, self->getWalletUid(), index);

                BitcoinLikeAccountDatabaseHelper::createAccount(sql, self->getWalletUid(), index, keychain->getRestoreKey());
                sql.commit();
                return std::static_pointer_cast<api::Account>(std::make_shared<BitcoinLikeAccount>(
                    self->shared_from_this(),
                    index,
                    self->_explorer,
                    self->_observer,
                    self->_synchronizerFactory(),
                    keychain
                ));
            });
        }

    }
}