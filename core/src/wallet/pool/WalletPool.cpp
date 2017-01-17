/*
 *
 * WalletPool
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/11/2016.
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
#include "../../utils/LambdaRunnable.hpp"
#include "WalletPool.hpp"
#include "WalletPoolBuilder.hpp"
#include "../../debug/LoggerApi.hpp"
#include "../../utils/Exception.hpp"

namespace ledger {
    namespace core {

        WalletPool::WalletPool( const std::string &name,
                                const std::experimental::optional<std::string> &password,
                                const std::shared_ptr<api::HttpClient> &httpClient,
                                const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                                const std::shared_ptr<api::PathResolver> &pathResolver,
                                const std::shared_ptr<api::LogPrinter> &logPrinter,
                                const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                                const std::shared_ptr<api::DatabaseBackend> &backend,
                                const std::unordered_map<std::string, std::string>& configuration) {

            _configuration = configuration;
            _resolver = pathResolver;
            // Initialize database
            _databaseBackend = std::dynamic_pointer_cast<DatabaseBackend>(backend);

            // Initialize threading objects
            _dispatcher = dispatcher;
            _queue = dispatcher->getSerialExecutionContext("pool_queue_" + name);

            // Initialize preferences
            _externalBackend = std::make_shared<PreferencesBackend>(
                    std::string("/") + name + "/preferences.db" ,
                    _queue,
                    _resolver
            );
            _internalBackend = std::make_shared<PreferencesBackend>(
                    std::string("/") + name + "/_preferences.db" ,
                    _queue,
                    _resolver
            );

            // Initialize logger
            _logger = logger::create(name + "-logs", password,
                                     dispatcher->getSerialExecutionContext("logger_queue_" + name),
                                     pathResolver, logPrinter);
            _loggerApi = std::make_shared<LoggerApi>(std::weak_ptr<spdlog::logger>(_logger));
            // Initialize network
            _http = std::make_shared<HttpClient>(_configuration[api::WalletPoolBuilder::API_BASE_URL], httpClient, _queue);

        }

        std::vector<std::shared_ptr<api::CryptoCurrencyDescription>> WalletPool::getAllSupportedCryptoCurrencies() {
            return std::vector<std::shared_ptr<api::CryptoCurrencyDescription>>();
        }

        std::shared_ptr<api::Logger> WalletPool::getLogger() {
            return _loggerApi;
        }

        void WalletPool::close() {
            runOnPoolQueue([] () -> void {

            });
        }

        void WalletPool::runOnPoolQueue(std::function<void()> func) {
            if (_queue != nullptr) {
                _queue->execute(LambdaRunnable::make(func));
            }
        }

        void WalletPool::runOnMainQueue(std::function<void()> func) {
            if (_queue != nullptr) {
                _queue->execute(LambdaRunnable::make(func));
            }
        }

        std::shared_ptr<api::Preferences> WalletPool::getPreferences() {
            return _externalBackend->getPreferences("pool");
        }

        std::shared_ptr<api::Preferences> WalletPool::getInterfacePreferences() {
            return _internalBackend->getPreferences("pool");
        }

        void WalletPool::open(const std::string &name, const std::experimental::optional<std::string> &password,
                              const std::shared_ptr<api::HttpClient> &httpClient,
                              const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                              const std::shared_ptr<api::PathResolver> &pathResolver,
                              const std::shared_ptr<api::LogPrinter> &logPrinter,
                              const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                              const std::shared_ptr<api::RandomNumberGenerator> &rng,
                              const std::shared_ptr<api::DatabaseBackend> &backend,
                              const std::unordered_map<std::string, std::string> &configuration,
                              const std::shared_ptr<api::WalletPoolBuildCallback> &listener) {
            auto queue = dispatcher->getSerialExecutionContext("pool_queue_" + name);
            queue->execute(LambdaRunnable::make([=] () {
                try {
                    auto pool = new WalletPool(
                            name, password, httpClient, webSocketClient, pathResolver, logPrinter, dispatcher, rng,
                            backend,
                            configuration
                    );
                    if (listener) {
                        listener->onWalletPoolBuilt(std::shared_ptr<WalletPool>(pool));
                    }
                } catch (std::exception& e) {
                    if (listener) {
                        listener->onWalletPoolBuildError(Exception(api::ErrorCode::RUNTIME_ERROR, e.what()).toApiError());
                    }
                }
            }));
        }

        void WalletPool::getBitcoinLikeWallet(const std::string &identifier,
                                              const std::shared_ptr<api::GetBitcoinLikeWalletCallback> &callback) {
            runOnPoolQueue([] () {

            });
        }

        void WalletPool::getAllBitcoinLikeWalletIdentifiers(const std::shared_ptr<api::StringArrayCallback> &callback) {
            runOnPoolQueue([this, callback] () {
               auto identifiers = getInterfacePreferences()->getStringArray("bitcoin_wallets", {});
                runOnMainQueue([callback, identifiers] () {
                   callback->onCallback(identifiers);
                });
            });
        }

        void WalletPool::getOrCreateBitcoinLikeWallet(
                const std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> &publicKeyProvider,
                const std::shared_ptr<api::CryptoCurrencyDescription> &currency,
                const std::shared_ptr<api::Configuration> &configuration,
                const std::shared_ptr<api::GetBitcoinLikeWalletCallback> &callback) {

        }

        void WalletPool::getWalletPreferences(const std::string &walletIdentifier) {

        }
    }
}