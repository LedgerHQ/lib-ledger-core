/*
 *
 * LedgerApiBitcoinLikeBlockchainObserver.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/10/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "LedgerApiBitcoinLikeBlockchainObserver.h"
#include <api/ConfigurationDefaults.hpp>
#include <api/Configuration.hpp>
#include <math/Fibonacci.h>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/explorers/api/WebSocketNotificationParser.h>

namespace ledger {
    namespace core {

        LedgerApiBitcoinLikeBlockchainObserver::LedgerApiBitcoinLikeBlockchainObserver(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<WebSocketClient> &client,
                const std::shared_ptr<api::DynamicObject>& configuration,
                const std::shared_ptr<spdlog::logger>& logger,
                const api::Currency &currency) :
            BitcoinLikeBlockchainObserver(context, configuration, logger, currency, {api::Configuration::BLOCKCHAIN_OBSERVER_WS_ENDPOINT}) {
            _client = client;
            auto baseUrl = getConfiguration()->getString(api::Configuration::BLOCKCHAIN_OBSERVER_WS_ENDPOINT)
                    .value_or(api::ConfigurationDefaults::BLOCKCHAIN_OBSERVER_WS_ENDPOINT);
            _url = fmt::format(baseUrl, getCurrency().bitcoinLikeNetworkParameters.value().Identifier);
        }

        void LedgerApiBitcoinLikeBlockchainObserver::onStart() {
          connect();
        }

        void LedgerApiBitcoinLikeBlockchainObserver::onStop() {
            auto self = shared_from_this();
            run([self] () {
                if (self->_socket != nullptr)
                    self->_socket->close();
                self->_handler = [self] (WebSocketEventType event,
                               const std::shared_ptr<WebSocketConnection>& connection,
                               const Option<std::string>& message, Option<api::ErrorCode> code) {

                };
                self->_socket = nullptr;
            });
        }

        void LedgerApiBitcoinLikeBlockchainObserver::connect() {
            logger()->info("Connect {} observer", getCurrency().name);
            auto self = shared_from_this();
            _handler = [self] (WebSocketEventType event,
                               const std::shared_ptr<WebSocketConnection>& connection,
                               const Option<std::string>& message, Option<api::ErrorCode> code) {
                self->onSocketEvent(event, connection, message, code);
            };
            _client->connect(_url, _handler);
        }

        void LedgerApiBitcoinLikeBlockchainObserver::reconnect() {
            auto self = shared_from_this();
            auto delay = std::min(std::max(Fibonacci::compute(_attempt) * 500, 30000), 500);
            logger()->info("Attempt reconnection in {}ms for {} observer", delay, getCurrency().name);
            getContext()->delay(LambdaRunnable::make([self] () {
                self->connect();
            }), delay);
        }

        void LedgerApiBitcoinLikeBlockchainObserver::onSocketEvent(WebSocketEventType event,
                                                                   const std::shared_ptr<WebSocketConnection> &connection,
                                                                   const Option<std::string> &message,
                                                                   Option<api::ErrorCode> code) {
            switch (event) {
                case WebSocketEventType::CONNECT:
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

        void LedgerApiBitcoinLikeBlockchainObserver::onMessage(const std::string &message) {
            auto self = shared_from_this();
            run([message, self] () {
                auto result = JSONUtils::parse<WebSocketNotificationParser>(message);
                result->block.currencyName = self->getCurrency().name;
                if (result->type == "new-transaction") {
                    self->putTransaction(result->transaction);
                } else if (result->type == "new-block") {
                    self->putBlock(result->block);
                }
            });
        }

    }
}