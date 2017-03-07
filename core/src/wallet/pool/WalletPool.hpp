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
#ifndef LEDGER_CORE_WALLETPOOL_HPP
#define LEDGER_CORE_WALLETPOOL_HPP

#include <memory>
#include <functional>
#include "../../api/WebSocketClient.hpp"
#include "../../api/HttpClient.hpp"
#include "../../api/WalletPool.hpp"
#include "../../api/ThreadDispatcher.hpp"
#include "../../api/PathResolver.hpp"
#include "../../api/LogPrinter.hpp"
#include "../../api/ExecutionContext.hpp"
#include "../../utils/optional.hpp"
#include "../../api/RandomNumberGenerator.hpp"
#include "../../database/DatabaseBackend.hpp"
#include "../../net/HttpClient.hpp"
#include "../../debug/logger.hpp"
#include "../../api/WalletPoolBuildCallback.hpp"
#include "../../api/Logger.hpp"
#include "../../debug/LoggerApi.hpp"
#include "../../preferences/PreferencesBackend.hpp"
#include "../../api/StringArrayCallback.hpp"
#include "../../api/BitcoinLikeExtendedPublicKeyProvider.hpp"
#include "../../api/BitcoinLikeExtendedPublicKey.hpp"
#include "../../api/BitcoinLikeWallet.hpp"
#include "../../api/BitcoinLikeWalletCallback.hpp"
#include <cereal/types/polymorphic.hpp>
#include "../../api_impl/ConfigurationImpl.hpp"
#include "../../async/DedicatedContext.hpp"
#include "../../preferences/Preferences.hpp"
#include "../../collections/collections.hpp"

namespace ledger {
    namespace core {

        class BitcoinLikeWalletFactory;

        class WalletPool : public api::WalletPool, DedicatedContext, public std::enable_shared_from_this<WalletPool> {

        public:
            struct WalletEntry {
                std::string walletIdentifier;
                std::string currencyIdentifier;
                ConfigurationImpl configuration;

                virtual void no_op() {

                };

                template <class Archive>
                void serialize(Archive& ar) {
                    ar(walletIdentifier, currencyIdentifier, configuration);
                }
            };

        private:
            WalletPool(
                    const std::string &name,
                    const std::experimental::optional<std::string>& password,
                    const std::shared_ptr<api::HttpClient> &httpClient,
                    const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                    const std::shared_ptr<api::PathResolver> &pathResolver,
                    const std::shared_ptr<api::LogPrinter> &logPrinter,
                    const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                    const std::shared_ptr<api::RandomNumberGenerator> &rng,
                    const std::shared_ptr<api::DatabaseBackend> &backend,
                    const std::unordered_map<std::string, std::string>& configuration
            );

        public:

            // Bitcoin
            void getOrCreateBitcoinLikeWallet(
                    const std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> &publicKeyProvider,
                    const api::BitcoinLikeNetworkParameters &networkParams,
                    const std::shared_ptr<api::Configuration> &configuration,
                    const std::shared_ptr<api::BitcoinLikeWalletCallback> &callback) override;

            void getBitcoinLikeWallet(const std::string &identifier,
                                      const std::shared_ptr<api::BitcoinLikeWalletCallback> &callback) override;

            void getSupportedBitcoinLikeNetworkParameters(
                    const std::shared_ptr<api::BitcoinLikeNetworkParametersCallback> &callback) override;

            void addBitcoinLikeNetworkParameters(const api::BitcoinLikeNetworkParameters &params) override;

            void removeBitcoinLikenetworkParameters(const api::BitcoinLikeNetworkParameters &params) override;

            // Utilities
            std::shared_ptr<api::Logger> getLogger() override;
            std::shared_ptr<api::Preferences> getPreferences() override;
            std::shared_ptr<api::Preferences> getWalletPreferences(const std::string &walletIdentifier) override;
            std::shared_ptr<api::Preferences>
            getAccountPreferences(const std::string &walletIdentifier, int32_t accountNumber) override;
            std::shared_ptr<api::Preferences> getOperationPreferences(const std::string &uid) override;
            std::shared_ptr<Preferences> getInternalPreferences();
            std::shared_ptr<Preferences> getExternalPreferences();

            ~WalletPool();

        private:
            std::shared_ptr<BitcoinLikeWalletFactory> getBitcoinLikeWalletFactory(const std::string& networkParamsIdentifier);

        private:
            std::shared_ptr<api::ThreadDispatcher> _dispatcher;
            std::shared_ptr<DatabaseBackend> _databaseBackend;
            std::shared_ptr<HttpClient> _http;
            std::unordered_map<std::string, std::string> _configuration;
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<api::Logger> _loggerApi;
            std::shared_ptr<api::PathResolver> _resolver;
            std::shared_ptr<api::RandomNumberGenerator> _rng;
            std::shared_ptr<api::WebSocketClient> _ws;
            std::experimental::optional<std::string> _password;
            std::shared_ptr<PreferencesBackend> _externalBackend;
            std::shared_ptr<PreferencesBackend> _internalBackend;

            // Bitcoin wallets
            std::unordered_map<std::string, std::shared_ptr<BitcoinLikeWalletFactory>> _bitcoinWalletFactories;
            std::vector<std::shared_ptr<api::BitcoinLikeWallet>> _bitcoinWallets;
            Map<std::string, api::BitcoinLikeNetworkParameters> _bitcoinNetworkParams;

        public:
            static void open( const std::string &name,
                              const std::experimental::optional<std::string> &password,
                              const std::shared_ptr<api::HttpClient> &httpClient,
                              const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                              const std::shared_ptr<api::PathResolver> &pathResolver,
                              const std::shared_ptr<api::LogPrinter> &logPrinter,
                              const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                              const std::shared_ptr<api::RandomNumberGenerator>& rng,
                              const std::shared_ptr<api::DatabaseBackend> &backend,
                              const std::unordered_map<std::string, std::string>& configuration,
                              const std::shared_ptr<api::WalletPoolBuildCallback>& listener);
        };
    }
}


#endif //LEDGER_CORE_WALLETPOOL_HPP
