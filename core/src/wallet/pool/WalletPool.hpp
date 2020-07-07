/*
 *
 * WalletPool
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/05/2017.
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
#ifndef LEDGER_CORE_WALLETPOOL_HPP
#define LEDGER_CORE_WALLETPOOL_HPP

#include <string>
#include <utils/Option.hpp>
#include <api/HttpClient.hpp>
#include <api/WebSocketClient.hpp>
#include <api/PathResolver.hpp>
#include <api/LogPrinter.hpp>
#include <api/ThreadDispatcher.hpp>
#include <api/RandomNumberGenerator.hpp>
#include <api/DatabaseBackend.hpp>
#include <api/WalletPoolCallback.hpp>

#include <memory>
#include <unordered_map>

#include <net/HttpClient.hpp>
#include <async/DedicatedContext.hpp>
#include <preferences/Preferences.hpp>
#include <api/DynamicObject.hpp>
#include <database/DatabaseSessionPool.hpp>
#include <collections/DynamicObject.hpp>
#include <wallet/bitcoin/factories/BitcoinLikeWalletFactory.hpp>
#include <wallet/common/AbstractWalletFactory.hpp>
#include <events/EventPublisher.hpp>
#include <net/WebSocketClient.h>
#include <utils/TTLCache.h>
namespace ledger {
    namespace core {
        class BitcoinLikeWalletFactory;

        class WalletPool : public DedicatedContext, public std::enable_shared_from_this<WalletPool> {
        public:
            std::shared_ptr<HttpClient> getHttpClient(const std::string& baseUrl);
            std::shared_ptr<WebSocketClient> getWebSocketClient() const;
            std::shared_ptr<Preferences> getExternalPreferences() const;
            std::shared_ptr<Preferences> getInternalPreferences() const;
            std::shared_ptr<api::PathResolver> getPathResolver() const;
            std::shared_ptr<api::RandomNumberGenerator> rng() const;
            std::shared_ptr<api::ThreadDispatcher> getDispatcher() const;
            std::shared_ptr<spdlog::logger> logger() const;
            std::shared_ptr<DatabaseSessionPool> getDatabaseSessionPool() const;
            std::shared_ptr<DynamicObject> getConfiguration() const;
            std::shared_ptr<api::EventBus> getEventBus() const;
            const std::string& getName() const;
            const std::string getPassword() const;

            std::shared_ptr<AbstractWalletFactory> getFactory(const std::string& currencyName) const;

            // Fetch wallet
            Future<int64_t> getWalletCount();
            Future<std::vector<std::shared_ptr<AbstractWallet>>> getWallets(int64_t from, int64_t size);
            FuturePtr<AbstractWallet> getWallet(const std::string& name);
            Future<api::ErrorCode> updateWalletConfig(const std::string &name,
                                                      const std::shared_ptr<api::DynamicObject> &configuration);
            Future<std::vector<std::string>> getWalletNames(int64_t from, int64_t size) const;

            // Create wallet
            FuturePtr<AbstractWallet> createWallet(const std::string& name,
                                                   const std::string& currencyName,
                                                   const std::shared_ptr<api::DynamicObject>& configuration);


            // Delete wallet
            Future<Unit> deleteWallet(const std::string& name);

            Future<api::Block> getLastBlock(const std::string& currencyName);
            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date);

            // Password management
            Future<api::ErrorCode> changePassword(
                const std::string& oldPassword,
                const std::string& newPassword
            );

            // Currencies management
            Option<api::Currency> getCurrency(const std::string& name) const;
            const std::vector<api::Currency>& getCurrencies() const;
            Future<Unit> addCurrency(const api::Currency& currency);
            Future<Unit> removeCurrency(const std::string& currencyName);
            static std::shared_ptr<WalletPool> newInstance(
                const std::string &name,
                const std::string &password,
                const std::shared_ptr<api::HttpClient> &httpClient,
                const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                const std::shared_ptr<api::PathResolver> &pathResolver,
                const std::shared_ptr<api::LogPrinter> &logPrinter,
                const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                const std::shared_ptr<api::DatabaseBackend> &backend,
                const std::shared_ptr<api::DynamicObject>& configuration,
                const std::shared_ptr<api::PreferencesBackend> &externalPreferencesBackend,
                const std::shared_ptr<api::PreferencesBackend> &internalPreferencesBackend
            );



            ~WalletPool() = default;

            /// Reset wallet pool.
            ///
            /// Resetting the wallet pool is an irreversible fresh reset of the whole wallet pool
            /// and all of its created (sub-)objects (wallets, accounts, transactions, etc.). Please
            /// consider a less destructive option before opting to use this. However, if you’re
            /// looking for a way to end up as if you were in a “fresh install” situation, this is
            /// the function to go to.
            ///
            /// Final warning: this function effectively swipes off everything. You’ve been warned.
            ///
            /// > Note: when calling that function, you must re-create a WalletPool as all objects
            /// > got destroyed. Consider restarting / exiting your application right after calling
            /// > that function. You are also highly advised to run that function on a code path
            /// > that doesn’t include having lots of objects in memory.
            Future<api::ErrorCode> freshResetAll();

            Option<api::Block> getBlockFromCache(const std::string &currencyName);
            std::shared_ptr<api::ExecutionContext> getThreadPoolExecutionContext() const;
        private:
            WalletPool(
                const std::string &name,
                const std::string &password,
                const std::shared_ptr<api::HttpClient> &httpClient,
                const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                const std::shared_ptr<api::PathResolver> &pathResolver,
                const std::shared_ptr<api::LogPrinter> &logPrinter,
                const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                const std::shared_ptr<api::DatabaseBackend> &backend,
                const std::shared_ptr<api::DynamicObject>& configuration,
                const std::shared_ptr<api::PreferencesBackend> &externalPreferencesBackend,
                const std::shared_ptr<api::PreferencesBackend> &internalPreferencesBackend
            );

            void initializeCurrencies();

            void createFactory(const api::Currency& currency);

            void initializeFactories();
            std::shared_ptr<AbstractWallet> buildWallet(const WalletDatabaseEntry& entry);

            static Option<WalletDatabaseEntry> getWalletEntryFromDatabase(const std::shared_ptr<WalletPool> &walletPool,
                                                                          const std::string &name);

            // General
            std::string _poolName;
            std::string _password;
            std::shared_ptr<DynamicObject> _configuration;

            // File system management
            std::shared_ptr<api::PathResolver> _pathResolver;

            // HTTP management
            std::shared_ptr<api::HttpClient> _httpEngine;
            std::unordered_map<std::string, std::weak_ptr<HttpClient>> _httpClients;

            // WS management
            std::shared_ptr<WebSocketClient> _wsClient;

            // Preferences management
            std::shared_ptr<api::PreferencesBackend> _externalPreferencesBackend;
            std::shared_ptr<api::PreferencesBackend> _internalPreferencesBackend;

            // Database management
            std::shared_ptr<DatabaseSessionPool> _database;

            // Logger
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<api::LogPrinter> _logPrinter;

            // Threading management
            std::shared_ptr<api::ThreadDispatcher> _threadDispatcher;

            // RNG management
            std::shared_ptr<api::RandomNumberGenerator> _rng;

            // Currencies management
            std::vector<api::Currency> _currencies;

            // Event publisher
            std::shared_ptr<EventPublisher> _publisher;

            // Factories management
            std::vector<std::shared_ptr<AbstractWalletFactory>> _factories;

            // Wallets
            std::unordered_map<std::string, std::shared_ptr<AbstractWallet>> _wallets;

            // Event filter variables
            std::mutex _eventFilterMutex;
            std::unordered_map<std::string, int64_t> _lastEmittedBlocks;

            std::shared_ptr<api::ExecutionContext> _threadPoolExecutionContext;
            //Here the key is the currency name
            TTLCache<std::string, api::Block> _blockCache;
        };
    }
}

#endif //LEDGER_CORE_WALLETPOOL_HPP
