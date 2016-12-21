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
#include "../../preferences/AtomicPreferencesBackend.hpp"
#include "../../utils/LambdaRunnable.hpp"
#include "WalletPool.hpp"
#include "WalletPoolBuilder.hpp"

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

            // Initialize database
            _databaseBackend = std::dynamic_pointer_cast<DatabaseBackend>(backend);

            // Initialize threading objects
            _dispatcher = dispatcher;
            _queue = dispatcher->getSerialExecutionContext("pool_queue_" + name);

            // Initialize preferences
            //_localPreferencesBackend = std::make_shared<AtomicPreferencesBackend>();

            // Initialize logger
            _logger = logger::create(name + "-logs", password,
                                     dispatcher->getSerialExecutionContext("logger_queue_" + name),
                                     pathResolver, logPrinter);

            // Initialize network
            _http = std::make_shared<HttpClient>(_configuration[api::WalletPoolBuilder::API_BASE_URL], httpClient, _queue);
        }

        void WalletPool::open(const std::function<void(bool)> &callback) {
            runOnPoolQueue([] () -> void {

            });
        }

        std::vector<std::shared_ptr<api::WalletCommonInterface>> WalletPool::getAllWallets() {
            return std::vector<std::shared_ptr<api::WalletCommonInterface>>();
        }

        std::vector<std::shared_ptr<api::BitcoinLikeWallet>> WalletPool::getAllBitcoinLikeWallets() {
            return std::vector<std::shared_ptr<api::BitcoinLikeWallet>>();
        }

        std::vector<std::shared_ptr<api::EthereumLikeWallet>> WalletPool::getAllEthereumLikeWallets() {
            return std::vector<std::shared_ptr<api::EthereumLikeWallet>>();
        }

        void WalletPool::getOrCreateBitcoinLikeWallet(
                const std::shared_ptr<api::BitcoinPublicKeyProvider> &publicKeyProvider,
                const std::shared_ptr<api::CryptoCurrencyDescription> &currency,
                const std::shared_ptr<api::GetBitcoinLikeWalletCallback> &callback) {

        }

        void WalletPool::getOrCreateEthereumLikeWallet(
                const std::shared_ptr<api::EthereumPublicKeyProvider> &publicKeyProvider,
                const std::shared_ptr<api::CryptoCurrencyDescription> &currency,
                const std::shared_ptr<api::GetEthreumLikeWalletCallback> &callback) {

        }

        std::vector<std::shared_ptr<api::CryptoCurrencyDescription>> WalletPool::getAllSupportedCryptoCurrencies() {
            return std::vector<std::shared_ptr<api::CryptoCurrencyDescription>>();
        }

        std::shared_ptr<api::Logger> WalletPool::getLogger() {
            return nullptr;
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

        std::shared_ptr<api::Preferences> WalletPool::getPreferences() {
            return _preferencesBackend->getPreferences("pool");
        }
    }
}