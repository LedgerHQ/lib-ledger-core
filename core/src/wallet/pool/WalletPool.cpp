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
#include "../../api/BitcoinLikeNetworkParameters.hpp"
#include "../../api/BitcoinLikeNetworkParametersCallback.hpp"
#include "../bitcoin/factories/BitcoinLikeWalletFactory.hpp"

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
                                const std::unordered_map<std::string, std::string>& configuration) :
                DedicatedContext(dispatcher->getSerialExecutionContext("pool_queue_" + name)) {

            _configuration = configuration;
            _resolver = pathResolver;
            // Initialize database
            _databaseBackend = std::dynamic_pointer_cast<DatabaseBackend>(backend);

            // Initialize threading objects
            _dispatcher = dispatcher;

            // Initialize preferences
            _externalBackend = std::make_shared<PreferencesBackend>(
                    std::string("/") + name + "/preferences.db" ,
                    _executionContext,
                    _resolver
            );
            _internalBackend = std::make_shared<PreferencesBackend>(
                    std::string("/") + name + "/_preferences.db" ,
                    _executionContext,
                    _resolver
            );

            // Initialize logger
            _logger = logger::create(name + "-logs", password,
                                     dispatcher->getSerialExecutionContext("logger_queue_" + name),
                                     pathResolver, logPrinter);
            _loggerApi = std::make_shared<LoggerApi>(std::weak_ptr<spdlog::logger>(_logger));
            // Initialize network
            _http = std::make_shared<HttpClient>(_configuration[api::WalletPoolBuilder::API_BASE_URL], httpClient, _executionContext);

            // Initialize Bitcoin constants
            BitcoinLikeWalletFactory::initialize(
                getExternalPreferences()->getSubPreferences("bitcoin_like")->getSubPreferences("networks"),
                _bitcoinNetworkParams
            );
            fmt::print("Bitcoin params {}\n",  _bitcoinNetworkParams["btc"].Identifier);
            fmt::print("Bitcoin contained {}\n",  _bitcoinNetworkParams.contains("btc"));
        }

        std::shared_ptr<api::Logger> WalletPool::getLogger() {
            return _loggerApi;
        }

        std::shared_ptr<api::Preferences> WalletPool::getPreferences() {
            return _externalBackend->getPreferences("pool");
        }

        std::shared_ptr<Preferences> WalletPool::getInternalPreferences() {
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

        void WalletPool::getOrCreateBitcoinLikeWallet(
                const std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> &publicKeyProvider,
                const api::BitcoinLikeNetworkParameters &networkParams,
                const std::shared_ptr<api::DynamicObject> &configuration,
                const std::shared_ptr<api::BitcoinLikeWalletCallback> &callback) {
            auto self = shared_from_this();
            async<std::shared_ptr<BitcoinLikeWalletFactory>>([self, networkParams, this] () {
                return getBitcoinLikeWalletFactory(networkParams.Identifier);
            })
            .flatMap<std::shared_ptr<BitcoinLikeWallet>>(_executionContext, [self, publicKeyProvider, this, configuration] (const std::shared_ptr<BitcoinLikeWalletFactory> &factory) {
                return factory->build(publicKeyProvider, configuration);
            }).callback(_dispatcher->getMainExecutionContext(), callback);
        }

        void WalletPool::getBitcoinLikeWallet(const std::string &identifier,
                                              const std::shared_ptr<api::BitcoinLikeWalletCallback> &callback) {
            async<std::shared_ptr<BitcoinLikeWalletFactory>>([=] () {
                auto wallets = getInternalPreferences()->getSubPreferences("wallets");
                auto entry = wallets->getObject<WalletEntry>(identifier);
                if (entry.isEmpty()) {
                    throw Exception(api::ErrorCode::WALLET_NOT_FOUND, "Wallet not found.");
                }
                return getBitcoinLikeWalletFactory(entry->currencyIdentifier);
            })
            .flatMap<std::shared_ptr<BitcoinLikeWallet>>(_executionContext, [=] (const std::shared_ptr<BitcoinLikeWalletFactory> &factory) {
                return factory->build(identifier);
            }).callback(_dispatcher->getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::Preferences> WalletPool::getWalletPreferences(const std::string &walletIdentifier) {
            return nullptr;
        }

        std::shared_ptr<api::Preferences>
        WalletPool::getAccountPreferences(const std::string &walletIdentifier, int32_t accountNumber) {
            return nullptr;
        }

        std::shared_ptr<api::Preferences> WalletPool::getOperationPreferences(const std::string &uid) {
            return nullptr;
        }

        std::shared_ptr<BitcoinLikeWalletFactory>
        WalletPool::getBitcoinLikeWalletFactory(const std::string &networkParamsIdentifier) {
            fmt::print("Bitcoin params {}\n",  _bitcoinNetworkParams["btc"].Identifier);
            fmt::print("Bitcoin contained {}\n",  _bitcoinNetworkParams.contains("btc"));
            if (_bitcoinWalletFactories.find(networkParamsIdentifier) != _bitcoinWalletFactories.end()) {
                return _bitcoinWalletFactories[networkParamsIdentifier];
            } else if (_bitcoinNetworkParams.contains(networkParamsIdentifier)) {
                return std::make_shared<BitcoinLikeWalletFactory>(
                        _bitcoinNetworkParams[networkParamsIdentifier],
                        shared_from_this(),
                        getExternalPreferences()->getSubPreferences("wallets")
                );
            }
            throw Exception(api::ErrorCode::UNKNOWN_NETWORK_PARAMETERS,
                            fmt::format("Unknown network parameters [{}]", networkParamsIdentifier)
            );
        }

        void WalletPool::addBitcoinLikeNetworkParameters(const api::BitcoinLikeNetworkParameters &params) {

        }

        void WalletPool::removeBitcoinLikenetworkParameters(const api::BitcoinLikeNetworkParameters &params) {

        }

        void WalletPool::getSupportedBitcoinLikeNetworkParameters(const std::shared_ptr<api::BitcoinLikeNetworkParametersCallback> &callback) {

        }

        std::shared_ptr<Preferences> WalletPool::getExternalPreferences() {
            return _externalBackend->getPreferences("pool");
        }

        WalletPool::~WalletPool() {
            fmt::print("Destruct pool\n");
        }

    }
}