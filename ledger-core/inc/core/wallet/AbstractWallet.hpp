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

#include <core/api/Account.hpp>
#include <core/api/AccountCreationInfo.hpp>
#include <core/api/Block.hpp>
#include <core/api/Currency.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/api/ExtendedKeyAccountCreationInfo.hpp>
#include <core/api/Wallet.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/collections/DynamicObject.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/debug/logger.hpp>
#include <core/events/EventPublisher.hpp>
#include <core/utils/DerivationScheme.hpp>
#include <core/preferences/Preferences.hpp>

namespace ledger {
    namespace core {
        class WalletPool;
        class AbstractAccount;
        class AbstractWallet : public virtual api::Wallet, public DedicatedContext, public virtual std::enable_shared_from_this<AbstractWallet> {
        public:
            AbstractWallet(
                const std::string& walletName,
                const api::Currency& currency,
                const std::shared_ptr<WalletPool>& pool,
                const std::shared_ptr<DynamicObject>& configuration,
                const DerivationScheme& derivationScheme
            );

            std::shared_ptr<api::EventBus> getEventBus() override;
            std::shared_ptr<api::Preferences> getPreferences() override;
            std::shared_ptr<api::Logger> getLogger() override;
            std::shared_ptr<api::Preferences> getAccountPreferences(int32_t index) override;

            api::Currency getCurrency() override;
            const api::Currency& getCurrency() const;
            std::string getName() override;

            void getNextAccountIndex(const std::function<void(std::experimental::optional<int32_t>, std::experimental::optional<api::Error>)> & callback) override;
            Future<int32_t> getNextAccountIndex();
            Future<int32_t> getAccountCount();
            void getAccountCount(const std::function<void(std::experimental::optional<int32_t>, std::experimental::optional<api::Error>)> & callback) override;

            void getLastBlock(const std::function<void(std::experimental::optional<api::Block>, std::experimental::optional<api::Error>)> & callback) override;
            Future<api::Block> getLastBlock();

            void getAccount(
                int32_t index,
                const std::function<void(std::shared_ptr<api::Account>, std::experimental::optional<api::Error>)> & callback
            ) override;
            FuturePtr<api::Account> getAccount(int32_t index);

            void getAccounts(
                int32_t offset,
                int32_t count,
                const std::function<void(std::experimental::optional<std::vector<std::shared_ptr<api::Account>>>, std::experimental::optional<api::Error>)> & callback
            ) override;
            Future<std::vector<std::shared_ptr<api::Account>>> getAccounts(int32_t offset, int32_t count);

            void getNextAccountCreationInfo(
                const std::function<void(std::experimental::optional<api::AccountCreationInfo>, std::experimental::optional<api::Error>)> & callback
            ) override;

            void getNextExtendedKeyAccountCreationInfo(
                const std::function<void(std::experimental::optional<api::ExtendedKeyAccountCreationInfo>, std::experimental::optional<api::Error>)> & callback
            ) override;

            void getAccountCreationInfo(
                int32_t accountIndex,
                const std::function<void(std::experimental::optional<api::AccountCreationInfo>, std::experimental::optional<api::Error>)> & callback
            ) override;

            void getExtendedKeyAccountCreationInfo(
                int32_t accountIndex,
                const std::function<void(std::experimental::optional<api::ExtendedKeyAccountCreationInfo>, std::experimental::optional<api::Error>)> & callback
            ) override;

            void newAccountWithInfo(
                const api::AccountCreationInfo & accountCreationInfo,
                const std::function<void(std::shared_ptr<api::Account>, std::experimental::optional<api::Error>)> & callback
            ) override;

            void newAccountWithExtendedKeyInfo(
                const api::ExtendedKeyAccountCreationInfo & extendedKeyAccountCreationInfo,
                const std::function<void(std::shared_ptr<api::Account>, std::experimental::optional<api::Error>)> & callback
            ) override;

            void eraseDataSince(
                const std::chrono::system_clock::time_point & date,
                const std::function<void(std::experimental::optional<api::ErrorCode>, std::experimental::optional<api::Error>)> & callback
            ) override ;
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
