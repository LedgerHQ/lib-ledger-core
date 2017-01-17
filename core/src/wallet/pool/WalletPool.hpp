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

namespace ledger {
    namespace core {

        class WalletPool : public api::WalletPool {
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

            void getOrCreateBitcoinLikeWallet(
                    const std::shared_ptr<api::BitcoinLikeExtendedPublicKeyProvider> &publicKeyProvider,
                    const std::shared_ptr<api::CryptoCurrencyDescription> &currency,
                    const std::shared_ptr<api::Configuration> &configuration,
                    const std::shared_ptr<api::GetBitcoinLikeWalletCallback> &callback) override;

            void getWalletPreferences(const std::string &walletIdentifier) override;

            void getAllBitcoinLikeWalletIdentifiers(const std::shared_ptr<api::StringArrayCallback> &callback) override;

            void getBitcoinLikeWallet(const std::string &identifier,
                                      const std::shared_ptr<api::GetBitcoinLikeWalletCallback> &callback) override;

            virtual std::vector<std::shared_ptr<api::CryptoCurrencyDescription>>
            getAllSupportedCryptoCurrencies() override;

            virtual std::shared_ptr<api::Logger> getLogger() override;

            virtual std::shared_ptr<api::Preferences> getPreferences() override;
            std::shared_ptr<api::Preferences> getInterfacePreferences();
            virtual void close() override;

        private:
            void runOnPoolQueue(std::function<void()> func);
            void runOnMainQueue(std::function<void ()> func);

        private:
            std::shared_ptr<api::ThreadDispatcher> _dispatcher;
            std::shared_ptr<api::ExecutionContext> _queue;
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
            std::vector<std::shared_ptr<api::BitcoinLikeWallet>> _wallet;

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
