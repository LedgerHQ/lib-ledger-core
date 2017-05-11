/*
 *
 * WalletPoolBuilder
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
#ifndef LEDGER_CORE_WALLETPOOLBUILDER_HPP
#define LEDGER_CORE_WALLETPOOLBUILDER_HPP

#include <unordered_map>

#include <api/WalletPoolBuilder.hpp>
#include <api/WalletPool.hpp>
#include <utils/optional.hpp>

namespace ledger {

    namespace core {
        class WalletPoolBuilder : public api::WalletPoolBuilder, protected std::enable_shared_from_this<WalletPoolBuilder> {

        public:
            virtual std::shared_ptr<api::WalletPoolBuilder> setPassword(const std::string &password) override;

            WalletPoolBuilder();

            virtual std::shared_ptr<api::WalletPoolBuilder>
            setHttpClient(const std::shared_ptr<api::HttpClient> &client) override;

            virtual std::shared_ptr<api::WalletPoolBuilder>
            setWebsocketClient(const std::shared_ptr<api::WebSocketClient> &client) override;

            virtual std::shared_ptr<api::WalletPoolBuilder>
            setPathResolver(const std::shared_ptr<api::PathResolver> &pathResolver) override;

            virtual std::shared_ptr<api::WalletPoolBuilder>
            setLogPrinter(const std::shared_ptr<api::LogPrinter> &printer) override;

            virtual std::shared_ptr<api::WalletPoolBuilder>
            setThreadDispatcher(const std::shared_ptr<api::ThreadDispatcher> &dispatcher) override;

            virtual std::shared_ptr<api::WalletPoolBuilder> setName(const std::string &name) override;

            virtual std::shared_ptr<api::WalletPoolBuilder>
            setRandomNumberGenerator(const std::shared_ptr<api::RandomNumberGenerator> &rng) override;

            virtual std::shared_ptr<api::WalletPoolBuilder>
            setDatabaseBackend(const std::shared_ptr<api::DatabaseBackend> &backend) override;

            virtual std::shared_ptr <api::WalletPoolBuilder>
            setConfiguration(const std::shared_ptr<api::DynamicObject> &configuration) override;

            virtual void build(const std::shared_ptr<api::WalletPoolCallback> &listener) override;

        private:
            std::shared_ptr<api::HttpClient> _httpClient;
            std::shared_ptr<api::WebSocketClient> _webSocketClient;
            std::shared_ptr<api::PathResolver> _pathResolver;
            std::shared_ptr<api::LogPrinter> _logPrinter;
            std::shared_ptr<api::ThreadDispatcher> _dispatcher;
            std::string _name;
            std::experimental::optional<std::string> _password;
            std::shared_ptr<api::RandomNumberGenerator> _rng;
            std::shared_ptr<api::DatabaseBackend> _backend;
            std::shared_ptr<api::DynamicObject> _configuration;
        };
    }
}


#endif //LEDGER_CORE_WALLETPOOLBUILDER_HPP
