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


#include "TezosLikeWallet.h"
#include "TezosLikeAccount.h"

#include <algorithm>

#include <async/wait.h>
#include <api/ErrorCode.hpp>
#include <api/AccountCallback.hpp>
#include <api/ConfigurationDefaults.hpp>
#include <api/KeychainEngines.hpp>

#include <tezos/TezosLikeExtendedPublicKey.h>

#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <api/TezosCurve.hpp>

namespace ledger {
    namespace core {

        const api::WalletType TezosLikeWallet::type = api::WalletType::ETHEREUM;

        TezosLikeWallet::TezosLikeWallet(const std::string &name,
                                         const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                                         const std::shared_ptr<TezosLikeBlockchainObserver> &observer,
                                         const std::shared_ptr<TezosLikeKeychainFactory> &keychainFactory,
                                         const TezosLikeAccountSynchronizerFactory &synchronizer,
                                         const std::shared_ptr<WalletPool> &pool,
                                         const api::Currency &network,
                                         const std::shared_ptr<DynamicObject> &configuration,
                                         const DerivationScheme &scheme
        )
                : AbstractWallet(name, network, pool, configuration, scheme) {
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
            if (info.chainCodes.size() != 1 || info.publicKeys.size() != 1 || info.owners.size() != 1)
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Account creation info are inconsistent (only one public key is needed)");
            auto self = getSelf();
            return async<api::ExtendedKeyAccountCreationInfo>([self, info]() -> api::ExtendedKeyAccountCreationInfo {
                if (info.owners.size() != info.derivations.size() || info.owners.size() != info.chainCodes.size() ||
                    info.publicKeys.size() != info.owners.size())
                    throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Account creation info are inconsistent (size of arrays differs)");
                api::ExtendedKeyAccountCreationInfo result;

                if (info.chainCodes[0].size() != 32 || info.publicKeys[0].size() != 33)
                    throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Account creation info are inconsistent (contains invalid public key(s))");
                DerivationPath occurencePath(info.derivations[0]);

                auto xpub = TezosLikeExtendedPublicKey::fromRaw(
                        self->getCurrency(),
                        Option<std::vector<uint8_t>>(),
                        info.publicKeys[0],
                        info.chainCodes[0],
                        info.derivations[0],
                        api::TezosCurve::SECP256K1
                );
                result.owners.push_back(info.owners[0]);
                result.derivations.push_back(info.derivations[0]);
                result.extendedKeys.push_back(xpub->toBase58());
                result.index = info.index;
                return result;
            }).flatMap<std::shared_ptr<ledger::core::api::Account>>(getContext(), [self](const api::ExtendedKeyAccountCreationInfo &info) {
                return self->newAccountWithExtendedKeyInfo(info);
            });
        }

        FuturePtr<ledger::core::api::Account>
        TezosLikeWallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {

            if (info.extendedKeys.empty()) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Empty extended keys passed to newAccountWithExtendedKeyInfo");
            }

            auto self = getSelf();
            auto scheme = getDerivationScheme();
            scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(info.index);
            auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
            auto index = info.index;
            return async<std::shared_ptr<api::Account> >([=]() -> std::shared_ptr<api::Account> {
                auto keychain = self->_keychainFactory->build(
                        index,
                        xpubPath,
                        getConfig(),
                        info,
                        getAccountInternalPreferences(index),
                        getCurrency()
                );
                soci::session sql(self->getDatabase()->getPool());
                soci::transaction tr(sql);
                auto accountUid = AccountDatabaseHelper::createAccountUid(self->getWalletUid(), index);
                if (AccountDatabaseHelper::accountExists(sql, self->getWalletUid(), index))
                    throw make_exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "Account {}, for wallet '{}', already exists", index, self->getWalletUid());
                AccountDatabaseHelper::createAccount(sql, self->getWalletUid(), index);
                TezosLikeAccountDatabaseHelper::createAccount(sql, self->getWalletUid(), index, info.extendedKeys[info.extendedKeys.size() - 1]);
                tr.commit();
                auto account = std::static_pointer_cast<api::Account>(
                        std::make_shared<TezosLikeAccount>(
                                self->shared_from_this(),
                                index,
                                self->_explorer,
                                self->_observer,
                                self->_synchronizerFactory(),
                                keychain
                        )
                );
                self->addAccountInstanceToInstanceCache(std::dynamic_pointer_cast<AbstractAccount>(account));
                return account;
            });
        }

        Future<api::ExtendedKeyAccountCreationInfo>
        TezosLikeWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
            auto self = std::dynamic_pointer_cast<TezosLikeWallet>(shared_from_this());
            return async<api::ExtendedKeyAccountCreationInfo>(
                    [self, accountIndex]() -> api::ExtendedKeyAccountCreationInfo {
                        api::ExtendedKeyAccountCreationInfo info;
                        info.index = accountIndex;
                        auto scheme = self->getDerivationScheme();
                        scheme.setCoinType(self->getCurrency().bip44CoinType).setAccountIndex(accountIndex);;
                        auto keychainEngine = self->getConfiguration()->getString(
                                api::Configuration::KEYCHAIN_ENGINE).value_or(
                                api::ConfigurationDefaults::DEFAULT_KEYCHAIN);
                        if (keychainEngine == api::KeychainEngines::BIP32_P2PKH ||
                            keychainEngine == api::KeychainEngines::BIP49_P2SH) {
                            auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
                            info.derivations.push_back(xpubPath.toString());
                            info.owners.push_back(std::string("main"));
                        } else {
                            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                                                 "No implementation found found for keychain {}", keychainEngine);
                        }

                        return info;
                    });
        }

        Future<api::AccountCreationInfo> TezosLikeWallet::getAccountCreationInfo(int32_t accountIndex) {
            auto self = std::dynamic_pointer_cast<TezosLikeWallet>(shared_from_this());
            return getExtendedKeyAccountCreationInfo(accountIndex)
                    .map<api::AccountCreationInfo>(getContext(), [self, accountIndex](const api::ExtendedKeyAccountCreationInfo info) {
                        api::AccountCreationInfo result;
                        result.index = accountIndex;
                        auto length = info.derivations.size();
                        for (auto i = 0; i < length; i++) {
                             result.derivations.push_back(DerivationPath(info.derivations[i]).toString());
                             result.owners.push_back(info.owners[i]);
                        }
                        return result;
            });
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
            auto keychain = _keychainFactory->restore(entry.index, xpubPath, getConfig(), entry.address,
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

    }
}