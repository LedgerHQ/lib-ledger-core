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

#pragma once

#include "api/Wallet.hpp"
#include <api/Currency.hpp>
#include <api/Account.hpp>
#include <preferences/Preferences.hpp>
#include <async/DedicatedContext.hpp>
#include <events/EventPublisher.hpp>
#include <debug/logger.hpp>
#include <api/WalletType.hpp>
#include <database/DatabaseSessionPool.hpp>
#include <collections/DynamicObject.hpp>
#include <utils/DerivationScheme.hpp>
#include <api/AccountCreationInfo.hpp>
#include <api/ExtendedKeyAccountCreationInfo.hpp>
#include <api/AccountCreationInfoCallback.hpp>
#include <api/ExtendedKeyAccountCreationInfoCallback.hpp>
#include <api/AccountCallback.hpp>
#include <api/Block.hpp>
#include <api/BlockCallback.hpp>
#include <api/DynamicObject.hpp>

namespace ledger {
    namespace core {
        class WalletPool;
        class AbstractAccount;
        class AbstractWallet : public virtual api::Wallet, public DedicatedContext, public virtual std::enable_shared_from_this<AbstractWallet> {
        public:
            AbstractWallet(const std::string& walletName,
                           const api::Currency& currency,
                           const std::shared_ptr<WalletPool>& pool,
                           const std::shared_ptr<DynamicObject>& configuration,
                           const DerivationScheme& derivationScheme
            );
            std::shared_ptr<api::EventBus> getEventBus() override;
            std::shared_ptr<api::Preferences> getPreferences() override;
            bool isInstanceOfBitcoinLikeWallet() override;
            bool isInstanceOfEthereumLikeWallet() override;
            bool isInstanceOfRippleLikeWallet() override;
            std::shared_ptr<api::Logger> getLogger() override;
            api::WalletType getWalletType() override;
            std::shared_ptr<api::Preferences> getAccountPreferences(int32_t index) override;
            std::shared_ptr<WalletPool> getPool() const;
            std::shared_ptr<api::BitcoinLikeWallet> asBitcoinLikeWallet() override;

            api::Currency getCurrency() override;
            const api::Currency& getCurrency() const;
            std::string getName() override;

            void getNextAccountIndex(const std::shared_ptr<api::I32Callback> &callback) override;
            Future<int32_t> getNextAccountIndex();
            Future<int32_t> getAccountCount();
            void getAccountCount(const std::shared_ptr<api::I32Callback> &callback) override;

            void getLastBlock(const std::shared_ptr<api::BlockCallback> &callback) override;
            Future<api::Block> getLastBlock();

            template <typename T>
            std::shared_ptr<T> asInstanceOf() {
                auto type = getWalletType();
                if (type == T::type) {
                    return std::dynamic_pointer_cast<T>(shared_from_this());
                }
                throw make_exception(api::ErrorCode::BAD_CAST, "Wallet of type {} cannot be cast to {}", api::to_string(type), api::to_string(T::type));
            };

            void getAccount(int32_t index, const std::shared_ptr<api::AccountCallback> &callback) override;
            FuturePtr<api::Account> getAccount(int32_t index);
            void
            getAccounts(int32_t offset, int32_t count, const std::shared_ptr<api::AccountListCallback> &callback) override;
            Future<std::vector<std::shared_ptr<api::Account>>> getAccounts(int32_t offset, int32_t count);

            void getNextAccountCreationInfo(const std::shared_ptr<api::AccountCreationInfoCallback> &callback) override;
            void getNextExtendedKeyAccountCreationInfo(
                    const std::shared_ptr<api::ExtendedKeyAccountCreationInfoCallback> &callback) override;

            void getAccountCreationInfo(int32_t accountIndex,
                                        const std::shared_ptr<api::AccountCreationInfoCallback> &callback) override;

            void getExtendedKeyAccountCreationInfo(int32_t accountIndex,
                                                   const std::shared_ptr<api::ExtendedKeyAccountCreationInfoCallback> &callback) override;

            void newAccountWithInfo(const api::AccountCreationInfo &accountCreationInfo,
                                    const std::shared_ptr<api::AccountCallback> &callback) override;

            void
            newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &extendedKeyAccountCreationInfo,
                                          const std::shared_ptr<api::AccountCallback> &callback) override;

            void eraseDataSince(const std::chrono::system_clock::time_point & date, const std::shared_ptr<api::ErrorCodeCallback> & callback) override ;
            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date);

            std::shared_ptr<api::DynamicObject> getConfiguration() override;

            virtual FuturePtr<api::Account> newAccountWithInfo(const api::AccountCreationInfo& info) = 0;
            virtual FuturePtr<api::Account> newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo& info) = 0;
            virtual Future<api::ExtendedKeyAccountCreationInfo> getExtendedKeyAccountCreationInfo(int32_t accountIndex) = 0;
            virtual Future<api::AccountCreationInfo> getAccountCreationInfo(int32_t accountIndex) = 0;
            virtual Future<api::AccountCreationInfo> getNextAccountCreationInfo();
            virtual Future<api::ExtendedKeyAccountCreationInfo> getNextExtendedKeyAccountCreationInfo();
        public:
            virtual std::shared_ptr<Preferences> getAccountExternalPreferences(int32_t index);
            virtual std::shared_ptr<Preferences> getAccountInternalPreferences(int32_t index);
            virtual std::shared_ptr<Preferences> getInternalPreferences() const;
            virtual std::shared_ptr<Preferences> getExternalPreferences() const;
            virtual std::shared_ptr<EventPublisher> getEventPublisher() const;
            virtual std::shared_ptr<spdlog::logger> logger() const;
            virtual std::shared_ptr<DatabaseSessionPool> getDatabase() const;
            virtual std::shared_ptr<api::ExecutionContext> getMainExecutionContext() const;
            virtual std::string getWalletUid() const;
            virtual std::shared_ptr<DynamicObject> getConfig() const;
            virtual const DerivationScheme& getDerivationScheme() const;

        protected:
            virtual std::shared_ptr<AbstractAccount> createAccountInstance(soci::session& sql, const std::string& accountUid) = 0;
            void addAccountInstanceToInstanceCache(const std::shared_ptr<AbstractAccount>& account);

        private:
            std::string _name;
            std::string _uid;
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<api::Logger> _loggerApi;
            std::shared_ptr<EventPublisher> _publisher;
            std::shared_ptr<Preferences> _externalPreferences;
            std::shared_ptr<Preferences> _internalPreferences;
            std::shared_ptr<DatabaseSessionPool> _database;
            api::Currency _currency;
            std::shared_ptr<api::ExecutionContext> _mainExecutionContext;
            std::shared_ptr<DynamicObject> _configuration;
            DerivationScheme _scheme;
            std::weak_ptr<WalletPool> _pool;
            std::unordered_map<int32_t, std::shared_ptr<AbstractAccount>> _accounts;

        };
    }
}
