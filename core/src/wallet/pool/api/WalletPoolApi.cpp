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

namespace ledger {
    namespace core {

        WalletPoolApi::WalletPoolApi(const std::shared_ptr<WalletPool> &pool) {
            _pool = pool;
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
            auto context = dispatcher->getSerialExecutionContext(fmt::format("pool_queue_{}", name));
            FuturePtr<WalletPoolApi>::async(context, [=] () {
                auto pool = std::make_shared<WalletPool>(
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
            }).callback(dispatcher->getMainExecutionContext(), listener);
        }
    }
}