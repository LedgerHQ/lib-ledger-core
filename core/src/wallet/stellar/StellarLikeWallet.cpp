/*
 *
 * StellarLikeWallet.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
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

#include "StellarLikeWallet.hpp"
#include <async/Promise.hpp>
#include "StellarLikeAccount.hpp"
#include "explorers/HorizonBlockchainExplorer.hpp"
#include "synchronizers/StellarLikeBlockchainExplorerAccountSynchronizer.hpp"
#include <wallet/common/database/AccountDatabaseHelper.h>
#include "database/StellarLikeAccountDatabaseHelper.hpp"
#include <api/BoolCallback.hpp>

namespace ledger {
    namespace core {

        const api::WalletType StellarLikeWallet::type = api::WalletType::STELLAR;

        StellarLikeWallet::StellarLikeWallet(const std::string &walletName, const api::Currency &currency,
                                             const std::shared_ptr<WalletPool> &pool,
                                             const std::shared_ptr<DynamicObject> &configuration,
                                             const DerivationScheme &derivationScheme,
                                             const StellarLikeWalletParams &params) :
                                             AbstractWallet(walletName, currency, pool, configuration, derivationScheme),
                                             _params(params) {

        }


        bool StellarLikeWallet::isSynchronizing() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::shared_ptr<api::EventBus> StellarLikeWallet::synchronize() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        bool StellarLikeWallet::isInstanceOfStellarLikeWallet() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::shared_ptr<api::StellarLikeWallet> StellarLikeWallet::asStellarLikeWallet() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        FuturePtr<api::Account>
        StellarLikeWallet::newAccountWithInfo(const api::AccountCreationInfo &info) {
            auto self = getSelf();
            return async<std::shared_ptr<api::Account>>([=] () -> std::shared_ptr<api::Account> {
                DerivationPath path(info.derivations[0]);
                if (info.publicKeys.size() < 1) {
                    throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Missing pubkey in account creation info.");
                }
                soci::session sql(self->getDatabase()->getPool());
                {
                    if (AccountDatabaseHelper::accountExists(sql, getWalletUid(), info.index)) {
                        throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Account {} already exists", info.index);
                    }
                    auto keychain = self->_params.keychainFactory->build(
                            info.index, path, std::dynamic_pointer_cast<DynamicObject>(self->getConfiguration()),
                            info, self->getAccountInternalPreferences(info.index), self->getCurrency()
                    );
                    soci::transaction tr(sql);
                    AccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index);
                    stellar::Account account;
                    account.accountIndex = info.index;
                    account.accountId = keychain->getAddress()->toString();
                    account.subentryCount = 0;
                    StellarLikeAccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index, account);
                    tr.commit();
                }
                return self->createAccountInstance(sql, AccountDatabaseHelper::createAccountUid(self->getWalletUid(), info.index));
            });
        }

        FuturePtr<ledger::core::api::Account>
        StellarLikeWallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "StellarLike doesn't support account creation through extended key info.");
        }

        Future<api::ExtendedKeyAccountCreationInfo>
        StellarLikeWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION,
                                 "StellarLike doesn't support account creation through extended key info.");
        }

        Future<api::AccountCreationInfo> StellarLikeWallet::getAccountCreationInfo(int32_t accountIndex) {
            auto scheme = getDerivationScheme();
            auto path = scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(accountIndex).getPath();
            api::AccountCreationInfo info {accountIndex, {"main"}, {path.toString()}, {}, {}};
            return Future<api::AccountCreationInfo>::successful(info);
        }

        std::shared_ptr<AbstractAccount>
        StellarLikeWallet::createAccountInstance(soci::session &sql, const std::string &accountUid) {
            stellar::Account account;
            StellarLikeAccountDatabaseHelper::getAccount(sql, accountUid, account);

            StellarLikeAccountParams params;
            params.index = account.accountIndex;
            auto scheme = getDerivationScheme();
            scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(account.accountIndex);
            auto path = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
            params.keychain = _params.keychainFactory->restore(
                    account.accountIndex, path, std::dynamic_pointer_cast<DynamicObject>(getConfiguration()),
                    account.accountId, getAccountInternalPreferences(account.accountIndex), getCurrency()
            );
            params.explorer = _params.blockchainExplorer;
            params.synchronizer = _params.accountSynchronizer;
            params.database = getDatabase();
            return std::make_shared<StellarLikeAccount>(getSelf(), params);
        }

        std::shared_ptr<StellarLikeWallet> StellarLikeWallet::getSelf() {
            return std::dynamic_pointer_cast<StellarLikeWallet>(shared_from_this());
        }

        void StellarLikeWallet::exists(const std::string &address, const std::shared_ptr<api::BoolCallback> &callback) {
            exists(address).onComplete(getContext(), [=] (const Try<bool>& result) {
                if (result.isFailure()) {
                    callback->onCallback(optional<bool>(), optional<api::Error>(api::Error(result.getFailure().getErrorCode(), result.getFailure().getMessage())));
                } else {
                    callback->onCallback(optional<bool>(result.getValue()), optional<api::Error>());
                }
            });
        }

        Future<bool> StellarLikeWallet::exists(const std::string &address) {
            auto explorer = _params.blockchainExplorer;
            StellarLikeAddress addr(address, getCurrency(), Option<std::string>::NONE);
            return explorer->getAccount(address).map<bool>(getContext(), [] (const auto& result) {
                return true;
            }).recover(getContext(), [] (const Exception& ex) {
                if (ex.getErrorCode() == api::ErrorCode::ACCOUNT_NOT_FOUND)
                    return false;
                throw ex;
            });
        }

    }
}
