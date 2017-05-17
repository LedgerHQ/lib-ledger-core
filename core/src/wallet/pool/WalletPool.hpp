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

namespace ledger {
    namespace core {
        class BitcoinLikeWalletFactory;

        class WalletPool : public DedicatedContext, public std::enable_shared_from_this<WalletPool> {
        public:
            std::shared_ptr<HttpClient> getHttpClient(const std::string& baseUrl);
            std::shared_ptr<Preferences> getExternalPreferences() const;
            std::shared_ptr<Preferences> getInternalPreferences() const;
            std::shared_ptr<api::PathResolver> getPathResolver() const;
            std::shared_ptr<api::RandomNumberGenerator> rng() const;
            std::shared_ptr<api::ThreadDispatcher> getDispatcher() const;
            std::shared_ptr<spdlog::logger> logger() const;
            std::shared_ptr<DatabaseSessionPool> getDatabaseSessionPool() const;
            std::shared_ptr<DynamicObject> getConfiguration() const;
            const std::string& getName() const;
            const Option<std::string> getPassword() const;
            const std::vector<api::Currency>& getCurrencies() const;

            std::shared_ptr<AbstractWalletFactory> getFactory(const std::string& currencyName) const;

            // Fetch wallet
            Future<int64_t> getWalletCount() const;
            Future<std::vector<std::shared_ptr<AbstractWallet>>> getWallets(int64_t from, int64_t size);
            FuturePtr<AbstractWallet> getWallet(const std::string& name);
            Future<std::vector<std::string>> getWalletNames(int64_t from, int64_t size) const;

            // Create wallet
            FuturePtr<AbstractWallet> createWallet(const std::string& name,
                                                   api::WalletType type,
                                                   const std::shared_ptr<api::DynamicObject>& configuration);


            // Delete wallet
            Future<Unit> deleteWallet(const std::string& name);


            static std::shared_ptr<WalletPool> newInstance(const std::string &name,
                                                           const Option<std::string> &password,
                                                           const std::shared_ptr<api::HttpClient> &httpClient,
                                                           const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                                                           const std::shared_ptr<api::PathResolver> &pathResolver,
                                                           const std::shared_ptr<api::LogPrinter> &logPrinter,
                                                           const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                                                           const std::shared_ptr<api::RandomNumberGenerator>& rng,
                                                           const std::shared_ptr<api::DatabaseBackend> &backend,
                                                           const std::shared_ptr<api::DynamicObject>& configuration);

        private:
            WalletPool(const std::string &name,
                       const Option<std::string> &password,
                       const std::shared_ptr<api::HttpClient> &httpClient,
                       const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                       const std::shared_ptr<api::PathResolver> &pathResolver,
                       const std::shared_ptr<api::LogPrinter> &logPrinter,
                       const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                       const std::shared_ptr<api::RandomNumberGenerator>& rng,
                       const std::shared_ptr<api::DatabaseBackend> &backend,
                       const std::shared_ptr<api::DynamicObject>& configuration);
            void initializeCurrencies();
            void initializeFactories();
            std::shared_ptr<AbstractWallet> buildWallet(const WalletDatabaseEntry& entry);

        private:
            // General
            std::string _poolName;
            Option<std::string> _password;
            std::shared_ptr<DynamicObject> _configuration;

            // File system management
            std::shared_ptr<api::PathResolver> _pathResolver;

            // HTTP management
            std::shared_ptr<api::HttpClient> _httpEngine;
            std::unordered_map<std::string, std::weak_ptr<HttpClient>> _httpClients;

            // WS management
            std::shared_ptr<api::WebSocketClient> _wsEngine;

            // Preferences management
            std::shared_ptr<PreferencesBackend> _externalPreferencesBackend;
            std::shared_ptr<PreferencesBackend> _internalPreferencesBackend;

            // Database management
            std::shared_ptr<DatabaseSessionPool> _database;

            // Logger
            std::shared_ptr<spdlog::logger> _logger;

            // Threading management
            std::shared_ptr<api::ThreadDispatcher> _threadDispatcher;

            // RNG management
            std::shared_ptr<api::RandomNumberGenerator> _rng;

            // Currencies management
            std::vector<api::Currency> _currencies;

            // Factories management
            std::vector<std::shared_ptr<AbstractWalletFactory>> _factories;

            // Wallets
            std::unordered_map<std::string, std::shared_ptr<AbstractWallet>> _wallets;
        };
    }
}


#endif //LEDGER_CORE_WALLETPOOL_HPP
