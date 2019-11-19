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

#include <algorithm>

#include <tezos/TezosLikeAccount.hpp>
#include <tezos/TezosLikeExtendedPublicKey.hpp>
#include <tezos/TezosLikeWallet.hpp>
#include <tezos/api/TezosCurve.hpp>
#include <tezos/api/TezosConfiguration.hpp>
#include <tezos/api/TezosConfigurationDefaults.hpp>
#include <tezos/database/TezosLikeAccountDatabaseHelper.hpp>

#include <core/Services.hpp>
#include <core/async/Wait.hpp>
#include <core/api/ErrorCode.hpp>
#include <core/api/ConfigurationDefaults.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>

namespace ledger {
    namespace core {

        TezosLikeWallet::TezosLikeWallet(const std::string &name,
                                         const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                                         const std::shared_ptr<TezosLikeBlockchainObserver> &observer,
                                         const std::shared_ptr<TezosLikeKeychainFactory> &keychainFactory,
                                         const TezosLikeAccountSynchronizerFactory &synchronizer,
                                         const std::shared_ptr<Services> &services,
                                         const api::Currency &network,
                                         const std::shared_ptr<DynamicObject> &configuration,
                                         const DerivationScheme &scheme
        )
                : AbstractWallet(name, network, services, configuration, scheme) {
            _explorer = explorer;
            _observer = observer;
            _keychainFactory = keychainFactory;
            _synchronizerFactory = synchronizer;
        }

        bool TezosLikeWallet::isSynchronizing() {
            return false;
        }

        std::shared_ptr<api::EventBus> TezosLikeWallet::synchronize() {
            return nullptr;
        }

        FuturePtr<ledger::core::api::Account>
        TezosLikeWallet::newAccountWithInfo(const api::AccountCreationInfo &info) {
            auto self = getSelf();
            return async<std::shared_ptr<api::Account>>([=] () -> std::shared_ptr<api::Account> {
                DerivationPath path(info.derivations[0]);
                if (info.publicKeys.size() < 1) {
                    throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Missing pubkey in account creation info.");
                }
                soci::session sql(getDatabase()->getPool());
                {
                    if (AccountDatabaseHelper::accountExists(sql, getWalletUid(), info.index)) {
                        throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Account {} already exists", info.index);
                    }
                    auto keychain = self->_keychainFactory->build(
                            path,
                            std::dynamic_pointer_cast<DynamicObject>(self->getConfiguration()),
                            info,
                            self->getAccountInternalPreferences(info.index),
                            self->getCurrency()
                    );
                    soci::transaction tr(sql);

                    if (AccountDatabaseHelper::accountExists(sql, self->getWalletUid(), info.index))
                        throw make_exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "Account {}, for wallet '{}', already exists", info.index, self->getWalletUid());
                    AccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index);
                    TezosLikeAccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index, hex::toString(info.publicKeys[0]));
                    tr.commit();
                }
                return self->createAccountInstance(sql, AccountDatabaseHelper::createAccountUid(self->getWalletUid(), info.index));
            });
        }

        FuturePtr<ledger::core::api::Account>
        TezosLikeWallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Tezos does not support account creation with extended key infos.");
        }

        Future<api::ExtendedKeyAccountCreationInfo>
        TezosLikeWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Tezos does not support account creation with extended key infos.");
        }

        Future<api::AccountCreationInfo> TezosLikeWallet::getAccountCreationInfo(int32_t accountIndex) {
            auto scheme = getDerivationScheme();
            auto path = scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(accountIndex).getPath();
            return Future<api::AccountCreationInfo>::successful(api::AccountCreationInfo{accountIndex, {"main"}, {path.toString()}, {}, {}});
        }

        std::shared_ptr<TezosLikeWallet> TezosLikeWallet::getSelf() {
            return std::dynamic_pointer_cast<TezosLikeWallet>(shared_from_this());
        }

        std::shared_ptr<AbstractAccount>
        TezosLikeWallet::createAccountInstance(soci::session &sql, const std::string &accountUid) {
            TezosLikeAccountDatabaseEntry entry;
            TezosLikeAccountDatabaseHelper::queryAccount(sql, accountUid, entry);
            auto scheme = getDerivationScheme();
            scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(entry.index);
            auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
            auto keychain = _keychainFactory->restore(xpubPath,
                                                      getConfig(),
                                                      entry.publicKey,
                                                      getAccountInternalPreferences(entry.index), getCurrency());
            auto account = std::make_shared<TezosLikeAccount>(shared_from_this(),
                                                              entry.index,
                                                              _explorer,
                                                              _observer,
                                                              _synchronizerFactory(),
                                                              keychain);
            account->addOriginatedAccounts(sql, entry.originatedAccounts);
            return account;
        }

        std::shared_ptr<TezosLikeBlockchainExplorer> TezosLikeWallet::getBlockchainExplorer() {
            return _explorer;
        }

        bool TezosLikeWallet::hasMultipleAddresses() const {
            // TODO check if XTZ accepts multiple addresses
            return false;
        }
    }
}
