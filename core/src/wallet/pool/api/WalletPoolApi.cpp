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
#include "WalletPoolApi.hpp"
#include <async/Future.hpp>
#include <api/WalletCallback.hpp>
#include <api/I32Callback.hpp>
#include <api/CurrencyListCallback.hpp>
#include <api/CurrencyCallback.hpp>
#include <api/WalletListCallback.hpp>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <utils/ToCallback.hpp>
#include <stlab/concurrency/concurrency.hpp>

namespace ledger {
    namespace core {
        std::shared_ptr<api::WalletPool>
        api::WalletPool::newInstance(
            const std::string &name, const optional<std::string> &password,
            const std::shared_ptr<api::HttpClient> &httpClient,
            const std::shared_ptr<api::WebSocketClient> &webSocketClient,
            const std::shared_ptr<api::PathResolver> &pathResolver,
            const std::shared_ptr<api::LogPrinter> &logPrinter,
            const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
            const std::shared_ptr<api::RandomNumberGenerator> &rng,
            const std::shared_ptr<api::DatabaseBackend> &backend,
            const std::shared_ptr<api::DynamicObject> &configuration
        ) {
            auto pool = ledger::core::WalletPool::newInstance(name, Option<std::string>(password), httpClient, webSocketClient, pathResolver, logPrinter, dispatcher, rng, backend, configuration);
            return std::make_shared<WalletPoolApi>(pool);
        }

        void WalletPoolApi::open(const std::string &name, const std::experimental::optional<std::string> &password,
                                 const std::shared_ptr<api::HttpClient> &httpClient,
                                 const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                                 const std::shared_ptr<api::PathResolver> &pathResolver,
                                 const std::shared_ptr<api::LogPrinter> &logPrinter,
                                 const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                                 const std::shared_ptr<api::RandomNumberGenerator> &rng,
                                 const std::shared_ptr<api::DatabaseBackend> &backend,
                                 const std::shared_ptr<api::DynamicObject>& configuration,
                                 const std::shared_ptr<api::WalletPoolCallback> &listener) {
            toCallbackPtr(
                stlab::async(stlab::default_executor, [=]() {
                    auto pool = ledger::core::WalletPool::newInstance(
                        name,
                        Option<std::string>(password),
                        httpClient,
                        webSocketClient,
                        pathResolver,
                        logPrinter,
                        dispatcher,
                        rng,
                        backend,
                        configuration
                    );
                    return std::make_shared<WalletPoolApi>(pool);
                    }),
                listener);
        }

        WalletPoolApi::WalletPoolApi(const std::shared_ptr<ledger::core::WalletPool> &pool) {
            _pool = pool;
            _logger = std::make_shared<LoggerApi>(pool->logger());
            _mainContext = _pool->getDispatcher()->getMainExecutionContext();
        }

        std::shared_ptr<api::Logger> WalletPoolApi::getLogger() {
            return _logger;
        }

        std::shared_ptr<api::Preferences> WalletPoolApi::getPreferences() {
            return _pool->getExternalPreferences();
        }

        void WalletPoolApi::getWalletCount(const std::shared_ptr<api::I32Callback> &callback) {
            toCallback(_pool->getWalletCount(), callback);
        }

        void WalletPoolApi::getWallet(const std::string &name, const std::shared_ptr<api::WalletCallback> &callback) {
            toCallbackPtr(_pool->getWallet(name), callback);
        }

        void WalletPoolApi::updateWalletConfig(const std::string &name,
                                               const std::shared_ptr<api::DynamicObject> &configuration,
                                               const std::shared_ptr<api::ErrorCodeCallback> &callback) {
            toErrorCodeCallback(_pool->updateWalletConfig(name, configuration), callback);
        }

        void WalletPoolApi::createWallet(const std::string &name, const api::Currency &currency,
                                         const std::shared_ptr<api::DynamicObject> &configuration,
                                         const std::shared_ptr<api::WalletCallback> &callback) {
            toCallbackPtr(_pool->createWallet(name, currency.name, configuration), callback);
        }

        void WalletPoolApi::getCurrencies(const std::shared_ptr<api::CurrencyListCallback> &callback) {
            toCallback(
                stlab::async(stlab::default_executor, [pool = _pool]() {
                    auto currencies = pool->getCurrencies();
                    return currencies;
                    }),
                callback);
        }

        void
        WalletPoolApi::getCurrency(const std::string &name, const std::shared_ptr<api::CurrencyCallback> &callback) {
            auto pool = _pool;
            toCallback(
                stlab::async(stlab::default_executor, [pool, name]() {
                    auto currency = pool->getCurrency(name);
                    if (currency.isEmpty()) {
                        throw make_exception(api::ErrorCode::CURRENCY_NOT_FOUND, "Currency '{}' doesn't exist", name);
                    }
                    return currency.getValue();
                    }),
                callback);
        }

        void WalletPoolApi::getWallets(int32_t from, int32_t size, const std::shared_ptr<api::WalletListCallback> &callback) {
            toCallback(
                _pool->getWallets(from, size)
                .then([](const std::vector<std::shared_ptr<AbstractWallet>> & wallets) {
                    std::vector<std::shared_ptr<api::Wallet>> res;
                    std::transform(wallets.begin(), wallets.end(), std::back_inserter(res), [](auto & x) {return x; });
                    return res;
                    }),
                callback);
        }

        std::string WalletPoolApi::getName() {
            return _pool->getName();
        }

        WalletPoolApi::~WalletPoolApi() {

        }

        std::shared_ptr<api::EventBus> WalletPoolApi::getEventBus() {
            return _pool->getEventBus();
        }

        void WalletPoolApi::getLastBlock(const std::string &currencyName,
                                         const std::shared_ptr<api::BlockCallback> &callback) {
            toCallback(_pool->getLastBlock(currencyName), callback);
        }

        void WalletPoolApi::eraseDataSince(const std::chrono::system_clock::time_point & date, const std::shared_ptr<api::ErrorCodeCallback> & callback) {
            toErrorCodeCallback(_pool->eraseDataSince(date), callback);
        }

        void WalletPoolApi::freshResetAll(const std::shared_ptr<api::ErrorCodeCallback>& callback) {
            toErrorCodeCallback(_pool->freshResetAll(), callback);
        }

        void WalletPoolApi::changePassword(const std::string &oldPassword,
                                           const std::string &newPassword,
                                           const std::shared_ptr<api::ErrorCodeCallback> & callback) {
            toErrorCodeCallback(_pool->changePassword(oldPassword, newPassword), callback);
        }
    }
}
