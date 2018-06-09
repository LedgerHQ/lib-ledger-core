/*
 *
 * AbstractWallet
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/04/2017.
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
#include "AbstractWallet.hpp"
#include <wallet/pool/WalletPool.hpp>
#include <debug/LoggerApi.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <api/I32Callback.hpp>
#include "AbstractAccount.hpp"
#include <api/AccountListCallback.hpp>
#include <async/algorithm.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>

namespace ledger {
    namespace core {
        AbstractWallet::AbstractWallet(const std::string &walletName,
                                       const api::Currency &currency,
                                       const std::shared_ptr<WalletPool> &pool,
                                       const std::shared_ptr<DynamicObject> &configuration,
                                       const DerivationScheme &derivationScheme
        )
                : DedicatedContext(
                pool->getDispatcher()->getSerialExecutionContext(fmt::format("wallet_{}", walletName))),
                  _scheme(derivationScheme) {
            _pool = pool;
            _name = walletName;
            _uid = WalletDatabaseEntry::createWalletUid(pool->getName(), _name);
            _currency = currency;
            _configuration = configuration;
            _externalPreferences = pool->getExternalPreferences()->getSubPreferences(
                    fmt::format("wallet_{}", walletName));
            _internalPreferences = pool->getInternalPreferences()->getSubPreferences(
                    fmt::format("wallet_{}", walletName));
            _publisher = std::make_shared<EventPublisher>(getContext());
            _logger = pool->logger();
            _loggerApi = std::make_shared<LoggerApi>(pool->logger());
            _database = pool->getDatabaseSessionPool();
            _mainExecutionContext = pool->getDispatcher()->getMainExecutionContext();
            _logger = pool->logger();
        }

        std::shared_ptr<api::EventBus> AbstractWallet::getEventBus() {
            return _publisher->getEventBus();
        }

        std::shared_ptr<api::Preferences> AbstractWallet::getPreferences() {
            return _externalPreferences;
        }

        bool AbstractWallet::isInstanceOfBitcoinLikeWallet() {
            return getWalletType() == api::WalletType::BITCOIN;
        }

        bool AbstractWallet::isInstanceOfEthereumLikeWallet() {
            return getWalletType() == api::WalletType::ETHEREUM;
        }

        bool AbstractWallet::isInstanceOfRippleLikeWallet() {
            return getWalletType() == api::WalletType::RIPPLE;
        }

        std::shared_ptr<Preferences> AbstractWallet::getAccountInternalPreferences(int32_t index) {
            return getInternalPreferences()->getSubPreferences(fmt::format("account_{}", index));
        }

        std::shared_ptr<EventPublisher> AbstractWallet::getEventPublisher() const {
            return _publisher;
        }

        std::shared_ptr<Preferences> AbstractWallet::getInternalPreferences() const {
            return _internalPreferences;
        }

        std::shared_ptr<Preferences> AbstractWallet::getExternalPreferences() const {
            return _externalPreferences;
        }

        std::shared_ptr<api::Logger> AbstractWallet::getLogger() {
            return _loggerApi;
        }

        api::WalletType AbstractWallet::getWalletType() {
            return _currency.walletType;
        }

        std::shared_ptr<spdlog::logger> AbstractWallet::logger() const {
            return _logger;
        }

        std::shared_ptr<Preferences> AbstractWallet::getAccountExternalPreferences(int32_t index) {
            return getExternalPreferences()->getSubPreferences(fmt::format("account_{}", index));;
        }

        std::shared_ptr<api::Preferences> AbstractWallet::getAccountPreferences(int32_t index) {
            return getAccountExternalPreferences(index);
        }

        std::shared_ptr<DatabaseSessionPool> AbstractWallet::getDatabase() const {
            return _database;
        }

        api::Currency AbstractWallet::getCurrency() {
            return _currency;
        }

        std::string AbstractWallet::getName() {
            return _name;
        }

        std::shared_ptr<api::BitcoinLikeWallet> AbstractWallet::asBitcoinLikeWallet() {
            return asInstanceOf<BitcoinLikeWallet>();
        }

        std::shared_ptr<api::ExecutionContext> AbstractWallet::getMainExecutionContext() const {
            return _mainExecutionContext;
        }

        Future<int32_t> AbstractWallet::getNextAccountIndex() {
            auto self = shared_from_this();
            return async<int32_t>([self]() {
                soci::session sql(self->getDatabase()->getPool());
                return AccountDatabaseHelper::computeNextAccountIndex(sql, self->getWalletUid());
            });
        }

        void AbstractWallet::getNextAccountIndex(const std::shared_ptr<api::I32Callback> &callback) {
            getNextAccountIndex().callback(getMainExecutionContext(), callback);
        }

        std::string AbstractWallet::getWalletUid() const {
            return _uid;
        }

        std::shared_ptr<DynamicObject> AbstractWallet::getConfiguration() const {
            return _configuration;
        }

        const DerivationScheme &AbstractWallet::getDerivationScheme() const {
            return _scheme;
        }

        const api::Currency &AbstractWallet::getCurrency() const {
            return _currency;
        }

        Future<int32_t> AbstractWallet::getAccountCount() {
            auto self = shared_from_this();
            return async<int32_t>([self]() -> int32_t {
                soci::session sql(self->getDatabase()->getPool());
                return AccountDatabaseHelper::getAccountsCount(sql, self->getWalletUid());
            });
        }

        void AbstractWallet::getAccountCount(const std::shared_ptr<api::I32Callback> &callback) {
            getAccountCount().callback(getMainExecutionContext(), callback);
        }

        void
        AbstractWallet::getNextAccountCreationInfo(const std::shared_ptr<api::AccountCreationInfoCallback> &callback) {
            this->getNextAccountCreationInfo().callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::getNextExtendedKeyAccountCreationInfo(
                const std::shared_ptr<api::ExtendedKeyAccountCreationInfoCallback> &callback) {
            this->getNextExtendedKeyAccountCreationInfo().callback(getMainExecutionContext(), callback);
        }

        Future<api::AccountCreationInfo> AbstractWallet::getNextAccountCreationInfo() {
            auto self = shared_from_this();
            return getNextAccountIndex().flatMap<api::AccountCreationInfo>(getContext(),
                                                                           [self](const int32_t &index) -> Future<api::AccountCreationInfo> {
                                                                               return self->getAccountCreationInfo(
                                                                                       index);
                                                                           });
        }

        Future<api::ExtendedKeyAccountCreationInfo> AbstractWallet::getNextExtendedKeyAccountCreationInfo() {
            auto self = shared_from_this();
            return getNextAccountIndex()
                    .flatMap<api::ExtendedKeyAccountCreationInfo>(getContext(),
          [self](const int32_t &index) -> Future<api::ExtendedKeyAccountCreationInfo> {
              return self->getExtendedKeyAccountCreationInfo(
                      index);
                  });
}

        void AbstractWallet::getAccountCreationInfo(int32_t accountIndex,
                                                    const std::shared_ptr<api::AccountCreationInfoCallback> &callback) {
            getAccountCreationInfo(accountIndex).callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex,
                                                               const std::shared_ptr<api::ExtendedKeyAccountCreationInfoCallback> &callback) {
            getExtendedKeyAccountCreationInfo(accountIndex).callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::newAccountWithInfo(const api::AccountCreationInfo &accountCreationInfo,
                                                const std::shared_ptr<api::AccountCallback> &callback) {
            newAccountWithInfo(accountCreationInfo).callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::newAccountWithExtendedKeyInfo(
                const api::ExtendedKeyAccountCreationInfo &extendedKeyAccountCreationInfo,
                const std::shared_ptr<api::AccountCallback> &callback) {
            newAccountWithExtendedKeyInfo(extendedKeyAccountCreationInfo).callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::getAccount(int32_t index, const std::shared_ptr<api::AccountCallback>& callback) {
            getAccount(index).callback(getMainExecutionContext(), callback);
        }

        FuturePtr<api::Account> AbstractWallet::getAccount(int32_t index) {
            soci::session sql(getDatabase()->getPool());
            return getAccount(sql, index);
        }

        FuturePtr<api::Account> AbstractWallet::getAccount(soci::session &sql, int32_t index) {
            auto self = shared_from_this();
            return async<std::shared_ptr<api::Account>>([self, index, &sql] () -> std::shared_ptr<api::Account> {
                auto it = self->_accounts.find(index);
                if (it != self->_accounts.end()) {
                    auto ptr = it->second;
                    if (ptr != nullptr)
                        return ptr;
                }

                if (!AccountDatabaseHelper::accountExists(sql, self->getWalletUid(), index)) {
                    throw make_exception(api::ErrorCode::ACCOUNT_NOT_FOUND, "Account {}, for wallet '{}', doesn't exist", index,  self->getName());
                }
                auto account = self->createAccountInstance(sql, AccountDatabaseHelper::createAccountUid(self->getWalletUid(), index));
                self->addAccountInstanceToInstanceCache(account);
                return account;
            });
        }

        void AbstractWallet::getAccounts(int32_t offset, int32_t count,
                                         const std::shared_ptr<api::AccountListCallback> &callback) {
            getAccounts(offset, count).callback(getMainExecutionContext(), callback);
        }

        Future<std::vector<std::shared_ptr<api::Account>>> AbstractWallet::getAccounts(int32_t offset, int32_t count) {
            auto self = shared_from_this();
            return async<Unit>([=] () -> Unit {
                return unit;
            }).flatMap<std::vector<std::shared_ptr<api::Account>>>(getContext(), [=] (const Unit&) -> Future<std::vector<std::shared_ptr<api::Account>>> {
                std::vector<Future<std::shared_ptr<api::Account>> > accounts;
                std::list<int32_t> indexes;
                soci::session sql(getDatabase()->getPool());
                AccountDatabaseHelper::getAccountsIndexes(sql, getWalletUid(), offset, count, indexes);
                for (auto& index : indexes) {
                    accounts.push_back(getAccount(sql, index));
                }
                return core::async::sequence(getMainExecutionContext(), accounts);
            });
        }

        void AbstractWallet::addAccountInstanceToInstanceCache(const std::shared_ptr<AbstractAccount> &account) {
            _accounts[account->getIndex()] = account;
            _publisher->relay(account->getEventBus());
        }

        std::shared_ptr<WalletPool> AbstractWallet::getPool() const {
            auto pool = _pool.lock();
            if (!pool)
                throw make_exception(api::ErrorCode::ILLEGAL_STATE, "Wallet pool was released");
            return pool;
        }

        Future<api::Block> AbstractWallet::getLastBlock() {
            auto self = shared_from_this();
            return async<api::Block>([self] () -> api::Block {
                soci::session sql(self->getDatabase()->getPool());
                auto block = BlockDatabaseHelper::getLastBlock(sql, self->getCurrency().name);
                if (block.isEmpty())
                    throw make_exception(api::ErrorCode::BLOCK_NOT_FOUND, "No block for this currency");
                return block.getValue();
            });
        }

        void AbstractWallet::getLastBlock(const std::shared_ptr<api::BlockCallback> &callback) {
            getLastBlock().callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::eraseDataSince(const std::chrono::system_clock::time_point & date) {
            soci::session sql(_database->getPool());
            sql << "DELETE FROM accounts WHERE wallet_uid = :wallet_uid created_at >= :date ", soci::use(getWalletUid()), soci::use(date);
            for (auto& account : _accounts) {
                account.second->eraseDataSince(date);
            }
        }

    }
}