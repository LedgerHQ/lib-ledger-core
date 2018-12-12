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
#include <algorithm>
#include <async/wait.h>

namespace ledger {
    namespace core {
        namespace bitcoin {
            const api::WalletType BitcoinLikeWallet::type = api::WalletType::BITCOIN;

            BitcoinLikeWallet::BitcoinLikeWallet(const std::string &name,
                const std::shared_ptr<TransactionBroadcaster>& transactionBroadcaster,
                const std::shared_ptr<BitcoinLikeBlockchainObserver> &observer,
                const std::shared_ptr<BitcoinLikeKeychainFactory> &keychainFactory,
                const std::shared_ptr<AccountSynchronizerFactory> &synchronizerFactory,
                const std::shared_ptr<WalletPool> &pool,
                const api::Currency &network,
                const std::shared_ptr<DynamicObject>& configuration,
                const DerivationScheme& scheme
            )
                : AbstractWallet(name, network, pool, configuration, scheme) {
                _transactionBroadcaster = transactionBroadcaster;
                _observer = observer;
                _keychainFactory = keychainFactory;
                _synchronizerFactory = synchronizerFactory;
            }

            bool BitcoinLikeWallet::isSynchronizing() {
                return false;
            }

            std::shared_ptr<api::EventBus> BitcoinLikeWallet::synchronize() {
                return nullptr;
            }

            FuturePtr<ledger::core::api::Account>
                BitcoinLikeWallet::newAccountWithInfo(const api::AccountCreationInfo &info) {
                // TODO: Update data structure to be able to do P2SH with mixed HD and Solo keys.
                // Right now we only handle Full HD P2SH wallet.
                // For each owner
                    // Get the pair of keys
                    // Create extended key
                    // Serialize and store
                auto self = getSelf();
                return async<api::ExtendedKeyAccountCreationInfo>([self, info]() -> api::ExtendedKeyAccountCreationInfo {
                    if (info.owners.size() != info.derivations.size() || info.owners.size() != info.chainCodes.size() ||
                        info.publicKeys.size() != info.owners.size())
                        throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Account creation info are inconsistent (size of arrays differs)");
                    api::ExtendedKeyAccountCreationInfo result;
                    std::set<std::string> ownersSet(info.owners.begin(), info.owners.end());
                    auto ownersIterator = ownersSet.begin();
                    auto ownersEndIterator = ownersSet.end();
                    auto size = info.owners.size();
                    while (ownersIterator != ownersEndIterator) {
                        int32_t firstOccurence = -1;
                        int32_t secondOccurence = -1;

                        for (auto i = 0; i < size; i++) {
                            if (info.owners[i] != *ownersIterator) continue;
                            if (info.publicKeys[i].size() != 33 && info.publicKeys[i].size() != 65)
                                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Account creation info are inconsistent (contains invalid public key(s))");
                            if (firstOccurence == -1) {
                                firstOccurence = i;
                            }
                            else {
                                secondOccurence = i;
                                break;
                            }
                        }
                        if (firstOccurence == -1 || secondOccurence == -1)
                            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Account creation info are inconsistent (missing derivation(s))");
                        DerivationPath firstOccurencePath(info.derivations[firstOccurence]);
                        DerivationPath secondOccurencePath(info.derivations[secondOccurence]);
                        if (secondOccurencePath.getParent() != firstOccurencePath) {
                            std::swap(firstOccurencePath, secondOccurencePath);
                            std::swap(firstOccurence, secondOccurence);
                        }
                        if (secondOccurencePath.getParent() != firstOccurencePath)
                            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Account creation info are inconsistent (wrong paths)");
                        auto xpub = BitcoinLikeExtendedPublicKey::fromRaw(
                            self->getCurrency(),
                            Option<std::vector<uint8_t>>(info.publicKeys[firstOccurence]).toOptional(),
                            info.publicKeys[secondOccurence],
                            info.chainCodes[secondOccurence],
                            info.derivations[secondOccurence]
                        );
                        result.owners.push_back(*ownersIterator);
                        result.derivations.push_back(info.derivations[secondOccurence]);
                        result.extendedKeys.push_back(xpub->toBase58());
                        ownersIterator++;
                    }
                    result.index = info.index;
                    return result;
                }).flatMap<std::shared_ptr<ledger::core::api::Account>>(getContext(), [self](const api::ExtendedKeyAccountCreationInfo& info) -> Future<std::shared_ptr<ledger::core::api::Account>> {
                    return self->newAccountWithExtendedKeyInfo(info);
                });
            }

            FuturePtr<ledger::core::api::Account>
                BitcoinLikeWallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {
                auto self = getSelf();
                auto scheme = getDerivationScheme();
                scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(info.index);
                auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
                auto index = info.index;
                return async<std::shared_ptr<api::Account> >([=]() -> std::shared_ptr<api::Account> {
                    auto keychain = self->_keychainFactory->build(
                        index,
                        xpubPath,
                        getConfiguration(),
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
                    BitcoinLikeAccountDatabaseHelper::createAccount(sql, self->getWalletUid(), index, keychain->getRestoreKey());
                    tr.commit();
                    auto account = std::static_pointer_cast<api::Account>(std::make_shared<BitcoinLikeAccount>(
                        self->shared_from_this(),
                        index,
                        self->_transactionBroadcaster,
                        self->_observer,
                        self->_synchronizerFactory->createAccountSynchronizer(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 6, 20, 20)));
                    self->addAccountInstanceToInstanceCache(std::dynamic_pointer_cast<AbstractAccount>(account));
                    return account;
                });
            }

            Future<api::ExtendedKeyAccountCreationInfo>
                BitcoinLikeWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
                auto self = std::dynamic_pointer_cast<BitcoinLikeWallet>(shared_from_this());
                return async<api::ExtendedKeyAccountCreationInfo>([self, accountIndex]() -> api::ExtendedKeyAccountCreationInfo {
                    api::ExtendedKeyAccountCreationInfo info;
                    info.index = accountIndex;
                    auto scheme = self->getDerivationScheme();
                    scheme.setCoinType(self->getCurrency().bip44CoinType).setAccountIndex(accountIndex);;
                    auto keychainEngine = self->getConfiguration()->getString(api::Configuration::KEYCHAIN_ENGINE).value_or(api::ConfigurationDefaults::DEFAULT_KEYCHAIN);
                    if (keychainEngine == api::KeychainEngines::BIP32_P2PKH ||
                        keychainEngine == api::KeychainEngines::BIP49_P2SH) {
                        auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
                        info.derivations.push_back(xpubPath.toString());
                        info.owners.push_back(std::string("main"));
                    }
                    else {
                        throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "No implementation found found for keychain {}", keychainEngine);
                    }

                    return info;
                });
            }

            Future<api::AccountCreationInfo> BitcoinLikeWallet::getAccountCreationInfo(int32_t accountIndex) {
                auto self = std::dynamic_pointer_cast<BitcoinLikeWallet>(shared_from_this());
                return getExtendedKeyAccountCreationInfo(accountIndex).map<api::AccountCreationInfo>(getContext(), [self, accountIndex](const api::ExtendedKeyAccountCreationInfo info) -> api::AccountCreationInfo {
                    api::AccountCreationInfo result;
                    result.index = accountIndex;
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

            std::shared_ptr<AbstractAccount>
                BitcoinLikeWallet::createAccountInstance(soci::session &sql, const std::string &accountUid) {
                BitcoinLikeAccountDatabaseEntry entry;
                BitcoinLikeAccountDatabaseHelper::queryAccount(sql, accountUid, entry);
                auto scheme = getDerivationScheme();
                scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(entry.index);
                auto xpubPath = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX).getPath();
                auto keychain = _keychainFactory->restore(entry.index, xpubPath, getConfiguration(), entry.xpub,
                    getAccountInternalPreferences(entry.index), getCurrency());
                return std::make_shared<BitcoinLikeAccount>(shared_from_this(),
                    entry.index,
                    _transactionBroadcaster,
                    _observer,
                    _synchronizerFactory->createAccountSynchronizer(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 6, 20, 20));
            }
        }
    }
}