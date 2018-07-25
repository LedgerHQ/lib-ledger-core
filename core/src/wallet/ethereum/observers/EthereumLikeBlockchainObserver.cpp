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


#include "EthereumLikeBlockchainObserver.h"
#include <api/Configuration.hpp>
#include <wallet/ethereum/EthereumLikeAccount.h>
#include <math/Fibonacci.h>
#include <utils/JSONUtils.h>
namespace ledger {
    namespace core {

        EthereumLikeBlockchainObserver::EthereumLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext>& context,
                                                                       const std::shared_ptr<api::DynamicObject>& configuration,
                                                                       const std::shared_ptr<spdlog::logger>& logger,
                                                                       const api::Currency& currency,
                                                                       const std::vector<std::string>& matchableKeys) :
                                                                       DedicatedContext(context), ConfigurationMatchable(matchableKeys) {

            _currency = currency;
            _configuration = configuration;
            setConfiguration(configuration);
            _logger = logger;

        }

        EthereumLikeBlockchainObserver::EthereumLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                                                       const std::shared_ptr<WebSocketClient>& client,
                                                                       const std::shared_ptr<api::DynamicObject>& configuration,
                                                                       const std::shared_ptr<spdlog::logger>& logger,
                                                                       const api::Currency &currency) : EthereumLikeBlockchainObserver(context, configuration, logger, currency, {api::Configuration::BLOCKCHAIN_OBSERVER_WS_ENDPOINT}){
            _client = client;
        }


        bool EthereumLikeBlockchainObserver::registerAccount(const std::shared_ptr<EthereumLikeAccount> &account) {
            std::lock_guard<std::mutex> lock(_lock);
            if (!_isRegistered(lock, account)) {
                bool needsStart = _accounts.empty();
                _accounts.push_front(account);
                if (needsStart)
                    onStart();
                return true;
            }
            return false;
        }

        bool EthereumLikeBlockchainObserver::unregisterAccount(const std::shared_ptr<EthereumLikeAccount> &account) {
            std::lock_guard<std::mutex> lock(_lock);
            if (_isRegistered(lock, account)) {
                bool needsStop = _accounts.size() == 1;
                _accounts.remove(account);
                if (needsStop)
                    onStop();
                return true;
            }
            return false;
        }

        bool EthereumLikeBlockchainObserver::isRegistered(const std::shared_ptr<EthereumLikeAccount> &account) {
            std::lock_guard<std::mutex> lock(_lock);
            return _isRegistered(lock, account);
        }

        bool EthereumLikeBlockchainObserver::_isRegistered(std::lock_guard<std::mutex> &lock,
                                                          const std::shared_ptr<EthereumLikeAccount> &account) {
            for (auto& acc : _accounts) {
                if (acc.get() == account.get())
                    return true;
            }
            return false;
        }

        const api::Currency &EthereumLikeBlockchainObserver::getCurrency() const {
            return _currency;
        }

        bool EthereumLikeBlockchainObserver::isObserving() const {
            std::lock_guard<std::mutex> lock(_lock);
            return _accounts.size() > 0;
        }

        std::shared_ptr<api::DynamicObject> EthereumLikeBlockchainObserver::getConfiguration() const {
            return _configuration;
        }

        std::shared_ptr<spdlog::logger> EthereumLikeBlockchainObserver::logger() const {
            return _logger;
        }

        void EthereumLikeBlockchainObserver::putTransaction(const EthereumLikeBlockchainExplorer::Transaction &tx) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto& account : _accounts) {
                account->run([account, tx] () {
                    soci::session sql(account->getWallet()->getDatabase()->getPool());
                    if (account->putTransaction(sql, tx) != EthereumLikeAccount::FLAG_TRANSACTION_IGNORED)
                        account->emitEventsNow();
                });
            }
        }

        void EthereumLikeBlockchainObserver::putBlock(const EthereumLikeBlockchainExplorer::Block &block) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto& account : _accounts) {
                account->run([account, block] () {
                    bool shouldEmitNow = false;
                    {
                        soci::session sql(account->getWallet()->getDatabase()->getPool());
                        shouldEmitNow = account->putBlock(sql, block);
                    }
                    if (shouldEmitNow)
                        account->emitEventsNow();
                });
            }
        }

        void EthereumLikeBlockchainObserver::onStart() {
            connect();
        }

        void EthereumLikeBlockchainObserver::onStop() {
            auto self = shared_from_this();
            run([self] () {
                if (self->_socket != nullptr)
                    self->_socket->close();
                self->_handler = [self] (WebSocketEventType event,
                                         const std::shared_ptr<WebSocketConnection>& connection,
                                         const Option<std::string>& message, Option<api::ErrorCode> code) {

                };
            });
        }

        void EthereumLikeBlockchainObserver::connect() {
            logger()->info("Connect {} observer", getCurrency().name);
            auto self = shared_from_this();
            _handler = [self] (WebSocketEventType event,
                               const std::shared_ptr<WebSocketConnection>& connection,
                               const Option<std::string>& message, Option<api::ErrorCode> code) {
                self->onSocketEvent(event, connection, message, code);
            };
            _client->connect(_url, _handler);
        }

        void EthereumLikeBlockchainObserver::reconnect() {
            auto self = shared_from_this();
            auto delay = std::min(std::max(Fibonacci::compute(_attempt) * 500, 30000), 500);
            logger()->info("Attempt reconnection in {}ms for {} observer", delay, getCurrency().name);
            getContext()->delay(LambdaRunnable::make([self] () {
                self->connect();
            }), delay);
        }

        void EthereumLikeBlockchainObserver::onSocketEvent(WebSocketEventType event,
                                                                   const std::shared_ptr<WebSocketConnection> &connection,
                                                                   const Option<std::string> &message,
                                                                   Option<api::ErrorCode> code) {
            switch (event) {
                case WebSocketEventType::CONNECT:
                    _socket = connection;
                    _attempt = 0;
                    logger()->info("Connected to websocket {}", _url);
                    break;
                case WebSocketEventType::RECEIVE:
                    onMessage(message.getValue());
                    break;
                case WebSocketEventType::CLOSE:
                    _attempt += 1;
                    _socket = nullptr;
                    if (code.hasValue())
                        logger()->error("An error occured to the connection with {}: {}", _url, message.getValue());
                    else
                        logger()->info("Close connection to {}", _url);
                    reconnect();
                    break;
            }
        }

        void EthereumLikeBlockchainObserver::onMessage(const std::string &message) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeBlockchainObserver::onMessage not implemented");
        }

    }
}