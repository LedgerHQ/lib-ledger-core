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
#include <api/PoolConfiguration.hpp>
#include <wallet/currencies.hpp>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>
#include "WalletPool.hpp"

namespace ledger {
    namespace core {

        WalletPool::WalletPool(const std::string &name, const Option<std::string> &password,
                               const std::shared_ptr<api::HttpClient> &httpClient,
                               const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                               const std::shared_ptr<api::PathResolver> &pathResolver,
                               const std::shared_ptr<api::LogPrinter> &logPrinter,
                               const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                               const std::shared_ptr<api::RandomNumberGenerator> &rng,
                               const std::shared_ptr<api::DatabaseBackend> &backend,
                               const std::shared_ptr<api::DynamicObject> &configuration)
        : DedicatedContext(dispatcher->getSerialExecutionContext(fmt::format("pool_queue_{}", name))) {
            // General
            _poolName = name;
            _password = password;
            _configuration = std::static_pointer_cast<DynamicObject>(configuration);

            // File system management
            _pathResolver = pathResolver;

            // HTTP management
            _httpEngine = httpClient;

            // WS management
            _wsEngine = webSocketClient;

            // Preferences management
            _externalPreferencesBackend = std::make_shared<PreferencesBackend>(
                fmt::format("/{}/preferences.db", _poolName),
                getContext(),
                _pathResolver
            );
            _internalPreferencesBackend = std::make_shared<PreferencesBackend>(
                fmt::format("/{}/__preferences__.db", _poolName),
                getContext(),
                _pathResolver
            );

            // Database management
            _database = std::make_shared<DatabaseSessionPool>(
               std::static_pointer_cast<DatabaseBackend>(backend),
               pathResolver,
               Option<std::string>(configuration->getString(api::PoolConfiguration::DATABASE_NAME)).getValueOr(name),
               DatabaseSessionPool::POOL_SIZE
            );

            // Logger management
            _logger = logger::create(
               name + "-l",
               password.toOptional(),
               dispatcher->getSerialExecutionContext(fmt::format("logger_queue_{}", name)),
               pathResolver,
               logPrinter
            );

            // Threading management
            _threadDispatcher = dispatcher;

            // Factories management
            //_bitcoinLikeFactory = std::make_shared<BitcoinLikeWalletFactory>(shared_from_this());

            // Initialization
            initializeCurrencies();
        }

        void WalletPool::initializeCurrencies() {
            soci::session sql(getDatabaseSessionPool()->getPool());
            sql.begin();
            for (auto& currency : currencies::ALL) {
                CurrenciesDatabaseHelper::insertCurrency(sql, currency);
            }
            sql.commit();
            CurrenciesDatabaseHelper::getAllCurrencies(sql, _currencies);
        }

        std::shared_ptr<Preferences> WalletPool::getExternalPreferences() const {
            return _externalPreferencesBackend->getPreferences("pool");
        }

        std::shared_ptr<Preferences> WalletPool::getInternalPreferences() const {
            return _internalPreferencesBackend->getPreferences("pool");
        }

        std::shared_ptr<spdlog::logger> WalletPool::logger() const {
            return _logger;
        }

        std::shared_ptr<DatabaseSessionPool> WalletPool::getDatabaseSessionPool() const {
            return _database;
        }

        std::shared_ptr<DynamicObject> WalletPool::getConfiguration() const {
            return _configuration;
        }

        const std::string &WalletPool::getName() const {
            return _poolName;
        }

        const Option<std::string> WalletPool::getPassword() const {
            return _password;
        }

        std::shared_ptr<api::PathResolver> WalletPool::getPathResolver() const {
            return _pathResolver;
        }

        std::shared_ptr<api::RandomNumberGenerator> WalletPool::rng() const {
            return _rng;
        }

        std::shared_ptr<api::ThreadDispatcher> WalletPool::getDispatcher() const {
            return _threadDispatcher;
        }

        std::shared_ptr<HttpClient> WalletPool::getHttpClient(const std::string &baseUrl) {
            auto it = _httpClients.find(baseUrl);
            if (it == _httpClients.end() || !it->second.lock()) {
                auto client = std::make_shared<HttpClient>(
                    baseUrl,
                    _httpEngine,
                    getDispatcher()->getThreadPoolExecutionContext(fmt::format("http_clients"))
                );
                _httpClients[baseUrl] = client;
            }
            return _httpClients[baseUrl].lock();
        }

        const std::vector<api::Currency> &WalletPool::getCurrencies() const {
            return _currencies;
        }

    }
}