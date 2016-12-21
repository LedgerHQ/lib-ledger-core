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
#include "WalletPoolBuilder.hpp"
#include "WalletPool.hpp"
#include "../../api/DatabaseBackend.hpp"

static const std::unordered_map<std::string, std::string> DEFAULT_CONFIGURATION = {
        {ledger::core::api::WalletPoolBuilder::API_BASE_URL, "https://api.ledgerwallet.com/blockchain/v2/"}
};

namespace ledger { namespace core {

        WalletPoolBuilder::WalletPoolBuilder() : _configuration(DEFAULT_CONFIGURATION) {
            _backend = api::DatabaseBackend::getSqlite3Backend();
        }

        std::shared_ptr<api::WalletPoolBuilder>
        WalletPoolBuilder::setHttpClient(const std::shared_ptr<api::HttpClient> &client) {
            _httpClient = client;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder>
        WalletPoolBuilder::setWebsocketClient(const std::shared_ptr<api::WebSocketClient> &client) {
            _webSocketClient = client;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder>
        WalletPoolBuilder::setPathResolver(const std::shared_ptr<api::PathResolver> &pathResolver) {
            _pathResolver = pathResolver;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder>
        WalletPoolBuilder::setLogPrinter(const std::shared_ptr<api::LogPrinter> &printer) {
            _logPrinter = printer;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder>
        WalletPoolBuilder::setThreadDispatcher(const std::shared_ptr<api::ThreadDispatcher> &dispatcher) {
            _dispatcher = dispatcher;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder> WalletPoolBuilder::setName(const std::string &name) {
            _name = name;
            return shared_from_this();
        }

        void WalletPoolBuilder::build(const std::shared_ptr<api::WalletPoolBuildCallback> &listener) {
            auto pool = std::make_shared<WalletPool>(
                    _name,
                    _password,
                    _httpClient,
                    _webSocketClient,
                    _pathResolver,
                    _logPrinter,
                    _dispatcher,
                    _rng,
                    _backend,
                    _configuration
            );
            pool->open([pool, listener] (bool isCreated) {
                listener->onWalletPoolBuilt(pool);
            });
        }

        std::shared_ptr<api::WalletPoolBuilder> WalletPoolBuilder::setPassword(const std::string &password) {
            _password = password;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder>
        WalletPoolBuilder::setRandomNumberGenerator(const std::shared_ptr<api::RandomNumberGenerator> &rng) {
            _rng = rng;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder>
        WalletPoolBuilder::setDatabaseBackend(const std::shared_ptr<api::DatabaseBackend> &backend) {
            _backend = backend;
            return shared_from_this();
        }

        std::shared_ptr<api::WalletPoolBuilder> WalletPoolBuilder::setConfiguration(const std::string &key, const std::string &value) {
            _configuration[key] = value;
            return shared_from_this();
        }


        std::shared_ptr<api::WalletPoolBuilder> api::WalletPoolBuilder::createInstance() {
            return std::make_shared<ledger::core::WalletPoolBuilder>();
        }
    }
}


