/*
 *
 * CosmosLikeWallet
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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


#include <cosmos/CosmosLikeWallet.hpp>

#include <algorithm>

#include <cosmos/CosmosLikeAccount.hpp>
#include <cosmos/database/CosmosLikeAccountDatabaseHelper.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <cosmos/api/CosmosCurve.hpp>

#include <core/async/Wait.hpp>
#include <core/api/ErrorCode.hpp>
#include <core/api/ConfigurationDefaults.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>

namespace ledger {
    namespace core {

        CosmosLikeWallet::CosmosLikeWallet(const std::string &name,
                                           const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
                                           const std::shared_ptr<CosmosLikeBlockchainObserver> &observer,
                                           const std::shared_ptr<CosmosLikeKeychainFactory> &keychainFactory,
                                           const CosmosLikeAccountSynchronizerFactory &synchronizer,
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

        bool CosmosLikeWallet::isSynchronizing() {
            return false;
        }

        std::shared_ptr<api::EventBus> CosmosLikeWallet::synchronize() {
            return nullptr;
        }

        FuturePtr<ledger::core::api::Account>
        CosmosLikeWallet::newAccountWithInfo(const api::AccountCreationInfo &info) {
            auto self = getSelf();
            return async<std::shared_ptr<api::Account>>([=] () {
               DerivationPath path(info.derivations[0]);
               if (info.publicKeys.size() == 0) {
                   throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Missing pubkey in account  creation info.");
               }
               soci::session sql(self->getDatabase()->getPool());
               {
                   if (AccountDatabaseHelper::accountExists(sql, self->getWalletUid(), info.index)) {
                       throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Account {} already exists", info.index);
                   }
                   auto keychain = self->_keychainFactory->build(path,
                                                                 std::dynamic_pointer_cast<DynamicObject>(self->getConfiguration()),
                                                                 info,
                                                                 self->getAccountInternalPreferences(info.index),
                                                                 self->getCurrency());
                   soci::transaction tr(sql);
                   AccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index);
                   CosmosLikeAccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index, keychain->getRestoreKey());
                   tr.commit();
               }
               return self->createAccountInstance(sql, AccountDatabaseHelper::createAccountUid(self->getWalletUid(), info.index));
            });
        }

        FuturePtr<ledger::core::api::Account>
        CosmosLikeWallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {
          throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION,
                               "CosmosLike doesn't support account creation "
                               "through extended key info.");
        }

        Future<api::ExtendedKeyAccountCreationInfo>
        CosmosLikeWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION,
                                 "CosmosLike doesn't support account creation "
                                 "through extended key info.");
        }

        Future<api::AccountCreationInfo> CosmosLikeWallet::getAccountCreationInfo(int32_t accountIndex) {
            auto scheme = getDerivationScheme();
            auto path = scheme.setCoinType(getCurrency().bip44CoinType)
                .setAccountIndex(accountIndex)
                .getPath();
            api::AccountCreationInfo info{
                                          accountIndex, {"main"}, {path.toString()}, {}, {}};
            return Future<api::AccountCreationInfo>::successful(info);
        }

        std::shared_ptr<CosmosLikeWallet> CosmosLikeWallet::getSelf() {
            return std::dynamic_pointer_cast<CosmosLikeWallet>(shared_from_this());
        }

        std::shared_ptr<AbstractAccount>
        CosmosLikeWallet::createAccountInstance(soci::session &sql, const std::string &accountUid) {
            CosmosLikeAccountDatabaseEntry entry;
            CosmosLikeAccountDatabaseHelper::queryAccount(sql, accountUid, entry);
            auto scheme = getDerivationScheme();
            scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(entry.index);
            auto path = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
            auto keychain = _keychainFactory->restore(path, getConfig(), entry.address,
                                                      getAccountInternalPreferences(entry.index), getCurrency());
            auto account = std::make_shared<CosmosLikeAccount>(shared_from_this(),
                                                               entry.index,
                                                               _explorer,
                                                               _observer,
                                                               _synchronizerFactory(),
                                                               keychain);
            return account;
        }

        std::shared_ptr<CosmosLikeBlockchainExplorer> CosmosLikeWallet::getBlockchainExplorer() {
            return _explorer;
        }

        bool CosmosLikeWallet::hasMultipleAddresses() const {
            // TODO check if ATOM accepts multiple addresses
            return false;
        }

    }
}
