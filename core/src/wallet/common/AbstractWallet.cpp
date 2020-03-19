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
#include <async/DedicatedContext.hpp>
#include <api/ConfigurationDefaults.hpp>
#include <wallet/stellar/StellarLikeAccount.hpp>
#include <wallet/stellar/StellarLikeWallet.hpp>

namespace ledger {
    namespace core {
        AbstractWallet::AbstractWallet(const std::string &walletName,
                                       const api::Currency &currency,
                                       const std::shared_ptr<WalletPool> &pool,
                                       const std::shared_ptr<DynamicObject> &configuration,
                                       const DerivationScheme &derivationScheme)
                : DedicatedContext(pool->getThreadPoolExecutionContext()),
                  _scheme(derivationScheme),
                  _balanceCache(std::chrono::seconds(configuration->getInt(api::Configuration::TTL_CACHE)
                                                             .value_or(api::ConfigurationDefaults::DEFAULT_TTL_CACHE)))
        {
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

        std::shared_ptr<DynamicObject> AbstractWallet::getConfig() const {
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
            return Future<int32_t>::async(getPool()->getThreadPoolExecutionContext(), [self]() -> int32_t {
                soci::session sql(self->getDatabase()->getPool());
                auto count = AccountDatabaseHelper::getAccountsCount(sql, self->getWalletUid());
                return count;
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
            auto self = shared_from_this();
            newAccountWithInfo(accountCreationInfo).callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::newAccountWithExtendedKeyInfo(
                const api::ExtendedKeyAccountCreationInfo &extendedKeyAccountCreationInfo,
                const std::shared_ptr<api::AccountCallback> &callback) {
            auto self = shared_from_this();
            newAccountWithExtendedKeyInfo(extendedKeyAccountCreationInfo).callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::getAccount(int32_t index, const std::shared_ptr<api::AccountCallback>& callback) {
            getAccount(index).callback(getMainExecutionContext(), callback);
        }

        FuturePtr<api::Account> AbstractWallet::getAccount(int32_t index) {
            auto it = _accounts.find(index);
            if (it != _accounts.end()) {
                auto ptr = it->second;
                if (ptr != nullptr)
                    return FuturePtr<api::Account>::successful(ptr);
            }
            auto self = shared_from_this();
            return FuturePtr<api::Account>::async(getPool()->getThreadPoolExecutionContext(), [self, index] () -> std::shared_ptr<api::Account> {
                soci::session sql(self->getDatabase()->getPool());
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
            return Future<std::vector<std::shared_ptr<api::Account>>>::async(getPool()->getThreadPoolExecutionContext(), [=] () {
                std::vector<Future<std::shared_ptr<api::Account>> > accounts;
                std::list<int32_t> indexes;
                soci::session sql(getDatabase()->getPool());
                AccountDatabaseHelper::getAccountsIndexes(sql, getWalletUid(), offset, count, indexes);
                for (auto& index : indexes) {
                    accounts.push_back(getAccount(index));
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
            return getPool()->getLastBlock(getCurrency().name);
        }

        void AbstractWallet::getLastBlock(const std::shared_ptr<api::BlockCallback> &callback) {
            getLastBlock().callback(getMainExecutionContext(), callback);
        }

        void AbstractWallet::eraseDataSince(const std::chrono::system_clock::time_point & date, const std::shared_ptr<api::ErrorCodeCallback> & callback) {
            eraseDataSince(date).callback(getMainExecutionContext(), callback);
        }

        Future<api::ErrorCode> AbstractWallet::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            auto self = shared_from_this();
            auto uid = getWalletUid();
            //auto accounts = _accounts;
            _logger->debug("Start erasing data of wallet : {} since : {}", uid, DateUtils::toJSON(date));
            static std::function<Future<api::ErrorCode> (int , const std::unordered_map<int32_t, std::shared_ptr<AbstractAccount>> &)> eraseAccount = [date] (int index, const std::unordered_map<int32_t, std::shared_ptr<AbstractAccount>> &accountsToErase) -> Future<api::ErrorCode> {

                if (index == accountsToErase.size()) {
                    return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
                }

                return accountsToErase.at(index)->eraseDataSince(date).flatMap<api::ErrorCode>(ImmediateExecutionContext::INSTANCE,[index, accountsToErase] (const api::ErrorCode &errorCode) -> Future<api::ErrorCode> {

                    if (errorCode != api::ErrorCode::FUTURE_WAS_SUCCESSFULL) {
                        return Future<api::ErrorCode>::failure(make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to erase accounts of wallet !"));
                    }

                    return eraseAccount(index + 1, accountsToErase);
                });
            };
            return eraseAccount(0, _accounts).flatMap<api::ErrorCode>(ImmediateExecutionContext::INSTANCE, [self, date, uid] (const api::ErrorCode &err) {

                if (err != api::ErrorCode::FUTURE_WAS_SUCCESSFULL) {
                    return Future<api::ErrorCode>::failure(make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to erase accounts of wallet {}", uid));
                }

                soci::session sql(self->getDatabase()->getPool());
                //Remove all accounts created after date
                soci::rowset<soci::row> accounts = (sql.prepare << "SELECT idx FROM accounts "
                                                                    "WHERE wallet_uid = :wallet_uid AND created_at >= :date",
                                                                    soci::use(uid), soci::use(date));

                for (auto& account : accounts) {
                    if (account.get_indicator(0) != soci::i_null) {
                        self->_accounts.erase(account.get<int32_t>(0));
                    }
                }
                sql << "DELETE FROM accounts WHERE wallet_uid = :wallet_uid AND created_at >= :date", soci::use(uid), soci::use(date);
                self->logger()->debug("Finish erasing data of wallet : {} since : {}",uid, DateUtils::toJSON(date));
                return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
            });

        }

        bool AbstractWallet::isInstanceOfStellarLikeWallet() const {
            return _currency.walletType == api::WalletType::STELLAR;
        }

        std::shared_ptr<api::StellarLikeWallet> AbstractWallet::asStellarLikeWallet() {
            return asInstanceOf<StellarLikeWallet>();
        }


        Option<Amount> AbstractWallet::getBalanceFromCache(size_t accountIndex) {
            return _balanceCache.get(fmt::format("{}-{}", _currency.name, accountIndex));
        }

        void AbstractWallet::updateBalanceCache(size_t accountIndex, Amount balance) {
            _balanceCache.put(fmt::format("{}-{}", _currency.name, accountIndex), balance);
        }

        std::shared_ptr<api::DynamicObject> AbstractWallet::getConfiguration() {
            return getConfig();
        }

    }
}
