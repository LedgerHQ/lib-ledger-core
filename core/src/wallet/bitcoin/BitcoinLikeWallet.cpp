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

        FuturePtr<ledger::core::api::Account>
        BitcoinLikeWallet::newAccountWithInfo(const api::AccountCreationInfo &info) {
            PromisePtr<api::Account> p;

            return p.getFuture();
        }

        FuturePtr<ledger::core::api::Account>
        BitcoinLikeWallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {
            auto self = getSelf();
            auto scheme = getDerivationScheme();
            scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(info.index);
            auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
            auto index = info.index;
            return _keychainFactory->build(getContext(),
                                    index,
                                    xpubPath,
                                    getConfiguration(),
                                    info,
                                    getAccountInternalPreferences(index),
                                    getCurrency()
            ).mapPtr<api::Account>(getContext(), [=] (const std::shared_ptr<BitcoinLikeKeychain>& keychain) -> std::shared_ptr<api::Account> {
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

        Future<api::ExtendedKeyAccountCreationInfo>
        BitcoinLikeWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeWallet>(shared_from_this());
            return async<api::ExtendedKeyAccountCreationInfo>([self, accountIndex] () -> api::ExtendedKeyAccountCreationInfo {
                api::ExtendedKeyAccountCreationInfo info;
                info.index = accountIndex;
                auto scheme = self->getDerivationScheme();
                scheme.setCoinType(self->getCurrency().bip44CoinType).setAccountIndex(accountIndex);;
                auto keychainEngine = self->getConfiguration()->getString(api::Configuration::KEYCHAIN_ENGINE).value_or(api::ConfigurationDefaults::DEFAULT_KEYCHAIN);
                if (keychainEngine == api::KeychainEngines::BIP32_P2PKH) {
                    auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
                    info.derivations.push_back(xpubPath.toString());
                    info.owners.push_back(std::string("main"));
                } else {
                    throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "No implementation found found for keychain {}", keychainEngine);
                }

                return info;
            });
        }

        Future<api::AccountCreationInfo> BitcoinLikeWallet::getAccountCreationInfo(int32_t accountIndex) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeWallet>(shared_from_this());
            return getExtendedKeyAccountCreationInfo(accountIndex).map<api::AccountCreationInfo>(getContext(), [self] (const api::ExtendedKeyAccountCreationInfo info) -> api::AccountCreationInfo {
                api::AccountCreationInfo result;
                auto length = info.derivations.size();
                for (auto i = 0; i < length; i++) {
                    DerivationPath path(info.derivations[i]);
                    auto owner = info.owners[i];
                    result.derivations.push_back(path.getParent().toString());
                    result.derivations.push_back(path.toString());
                    result.owners.push_back(owner);
                    result.owners.push_back(owner);
                }
                return result;
            });
        }

        std::shared_ptr<BitcoinLikeWallet> BitcoinLikeWallet::getSelf() {
            return std::dynamic_pointer_cast<BitcoinLikeWallet>(shared_from_this());
        }

    }
}