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
#ifndef LEDGER_CORE_WALLETPOOL_API_HPP
#define LEDGER_CORE_WALLETPOOL_API_HPP

#include <api/WalletPool.hpp>
#include <wallet/pool/WalletPool.hpp>
#include <api/WalletPoolCallback.hpp>
#include <debug/LoggerApi.hpp>

namespace ledger {
    namespace core {

        class BitcoinLikeWalletFactory;

        class WalletPoolApi : public api::WalletPool, public std::enable_shared_from_this<WalletPoolApi> {
        public:
            WalletPoolApi(const std::shared_ptr<ledger::core::WalletPool>& pool);
            std::shared_ptr<api::Logger> getLogger() override;
            std::shared_ptr<api::Preferences> getPreferences() override;

            void getWalletCount(const std::shared_ptr<api::I32Callback> &callback) override;

            void getWallet(const std::string &name, const std::shared_ptr<api::WalletCallback> &callback) override;

            void getWallets(int32_t from, int32_t size, const std::shared_ptr<api::WalletListCallback> &callback) override;

            void createWallet(const std::string &name, const api::Currency &currency,
                              const std::shared_ptr<api::DynamicObject> &configuration,
                              const std::shared_ptr<api::WalletCallback> &callback) override;

            void getCurrencies(const std::shared_ptr <api::CurrencyListCallback> &callback) override;

            void getCurrency(const std::string &name, const std::shared_ptr<api::CurrencyCallback> &callback) override;

            std::string getName() override;

            std::shared_ptr<api::EventBus> getEventBus() override;

            void
            getLastBlock(const std::string &currencyName, const std::shared_ptr<api::BlockCallback> &callback) override;

            ~WalletPoolApi();

        private:
            std::shared_ptr<ledger::core::WalletPool> _pool;
            std::shared_ptr<LoggerApi> _logger;
            std::shared_ptr<api::ExecutionContext> _mainContext;

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
                              const std::shared_ptr<api::DynamicObject>& configuration,
                              const std::shared_ptr<api::WalletPoolCallback>& listener);
        };
    }
}


#endif //LEDGER_CORE_WALLETPOOL_API_HPP
