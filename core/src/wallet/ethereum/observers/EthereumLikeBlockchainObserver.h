/*
 *
 * EthereumLikeBlockchainObserver
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#ifndef LEDGER_CORE_ETHEREUMLIKEBLOCKCHAINOBSERVER_H
#define LEDGER_CORE_ETHEREUMLIKEBLOCKCHAINOBSERVER_H

#include <string>
#include <list>

//#include <wallet/ethereum/EthereumLikeAccount.h>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>

#include <utils/ConfigurationMatchable.h>
#include <async/DedicatedContext.hpp>

#include <api/ExecutionContext.hpp>
#include <api/Currency.hpp>
#include <api/DynamicObject.hpp>

#include <net/WebSocketClient.h>
#include <net/WebSocketConnection.h>

#include <spdlog/logger.h>

namespace ledger {
    namespace core {
        class EthereumLikeAccount;
        class EthereumLikeBlockchainObserver : public DedicatedContext, public ConfigurationMatchable, public std::enable_shared_from_this<EthereumLikeBlockchainObserver> {
        public:
            EthereumLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext>& context,
                                          const std::shared_ptr<api::DynamicObject>& configuration,
                                          const std::shared_ptr<spdlog::logger>& logger,
                                          const api::Currency& currency,
                                          const std::vector<std::string>& matchableKeys);

            EthereumLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                           const std::shared_ptr<WebSocketClient>& client,
                                           const std::shared_ptr<api::DynamicObject>& configuration,
                                           const std::shared_ptr<spdlog::logger>& logger,
                                           const api::Currency &currency);

            bool registerAccount(const std::shared_ptr<EthereumLikeAccount> &account);

            bool unregisterAccount(const std::shared_ptr<EthereumLikeAccount> &account);

            bool isRegistered(const std::shared_ptr<EthereumLikeAccount> &account);

            const api::Currency &getCurrency() const;

            bool isObserving() const;

            std::shared_ptr<api::DynamicObject> getConfiguration() const;

            std::shared_ptr<spdlog::logger> logger() const;

        protected:
            void onStart();
            void onStop();

            void putTransaction(const EthereumLikeBlockchainExplorer::Transaction &tx);
            void putBlock(const EthereumLikeBlockchainExplorer::Block &block);
        private:

            void connect();
            void reconnect();
            void onSocketEvent(WebSocketEventType event,
                               const std::shared_ptr<WebSocketConnection>& connection,
                               const Option<std::string>& message, Option<api::ErrorCode> code);
            void onMessage(const std::string& message);

            bool _isRegistered(std::lock_guard<std::mutex> &lock,
                               const std::shared_ptr<EthereumLikeAccount> &account);
            mutable std::mutex _lock;
            std::list<std::shared_ptr<EthereumLikeAccount>> _accounts;
            api::Currency _currency;
            std::shared_ptr<api::DynamicObject> _configuration;
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<WebSocketClient> _client;
            std::shared_ptr<WebSocketConnection> _socket;
            WebSocketEventHandler _handler;
            int32_t _attempt;
            std::string _url;

        };
    }
}


#endif //LEDGER_CORE_ETHEREUMLIKEBLOCKCHAINOBSERVER_H
