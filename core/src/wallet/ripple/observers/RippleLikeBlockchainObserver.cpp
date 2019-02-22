/*
 *
 * RippleLikeBlockchainObserver
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#include "RippleLikeBlockchainObserver.h"
#include <api/ConfigurationDefaults.hpp>
#include <api/RippleConfigurationDefaults.hpp>

#include <wallet/ripple/RippleLikeAccount.h>
#include <math/Fibonacci.h>
#include <utils/JSONUtils.h>

#include <wallet/ripple/explorers/api/RippleLikeWebSocketNotificationParser.h>

namespace ledger {
    namespace core {

        RippleLikeBlockchainObserver::RippleLikeBlockchainObserver(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<api::DynamicObject> &configuration,
                const std::shared_ptr<spdlog::logger> &logger,
                const api::Currency &currency,
                const std::vector<std::string> &matchableKeys) :
                DedicatedContext(context), ConfigurationMatchable(matchableKeys) {

            _currency = currency;
            _configuration = configuration;
            setConfiguration(configuration);
            setLogger(logger);
        }

        RippleLikeBlockchainObserver::RippleLikeBlockchainObserver(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<WebSocketClient> &client,
                const std::shared_ptr<api::DynamicObject> &configuration,
                const std::shared_ptr<spdlog::logger> &logger,
                const api::Currency &currency) :
                RippleLikeBlockchainObserver(context, configuration, logger, currency,
                                             {api::RippleConfigurationDefaults::RIPPLE_OBSERVER_WS_ENDPOINT_S2}) {
            _client = client;
            _url = getConfiguration()->getString(api::Configuration::BLOCKCHAIN_OBSERVER_WS_ENDPOINT)
                    .value_or(api::RippleConfigurationDefaults::RIPPLE_OBSERVER_WS_ENDPOINT_S2);
        }

        void RippleLikeBlockchainObserver::putTransaction(const RippleLikeBlockchainExplorerTransaction &tx) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto &account : _accounts) {
                account->run([account, tx]() {
                    soci::session sql(account->getWallet()->getDatabase()->getPool());
                    if (account->putTransaction(sql, tx) != RippleLikeAccount::FLAG_TRANSACTION_IGNORED)
                        account->emitEventsNow();
                });
            }
        }

        void RippleLikeBlockchainObserver::putBlock(const RippleLikeBlockchainExplorer::Block &block) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto &account : _accounts) {
                account->run([account, block]() {
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


        void RippleLikeBlockchainObserver::onStart() {
            connect();
        }

        void RippleLikeBlockchainObserver::onStop() {
            auto self = shared_from_this();
            run([self]() {
                if (self->_socket != nullptr)
                    self->_socket->close();
                self->_handler = [self](WebSocketEventType event,
                                        const std::shared_ptr<WebSocketConnection> &connection,
                                        const Option<std::string> &message, Option<api::ErrorCode> code) {

                };
            });
        }

        void RippleLikeBlockchainObserver::connect() {
            RippleBlockchainObserver::logger()->info("Connect {} observer", getCurrency().name);
            auto self = shared_from_this();
            _handler = [self](WebSocketEventType event,
                              const std::shared_ptr<WebSocketConnection> &connection,
                              const Option<std::string> &message, Option<api::ErrorCode> code) {
                self->onSocketEvent(event, connection, message, code);
            };
            _client->connect(_url, _handler);
        }

        void RippleLikeBlockchainObserver::reconnect() {
            auto self = shared_from_this();
            auto delay = std::min(std::max(Fibonacci::compute(_attempt) * 500, 30000), 500);
            RippleBlockchainObserver::logger()->info("Attempt reconnection in {}ms for {} observer", delay,
                                                     getCurrency().name);
            getContext()->delay(LambdaRunnable::make([self]() {
                self->connect();
            }), delay);
        }


        void RippleLikeBlockchainObserver::onMessage(const std::string &message) {
            auto self = shared_from_this();
            run([message, self]() {
                auto result = JSONUtils::parse<RippleLikeWebSocketNotificationParser>(message);
                result->block.currencyName = self->getCurrency().name;
                if (result->type == "transaction") {
                    self->putTransaction(result->transaction);
                } else if (result->type == "ledgerClosed") {
                    self->putBlock(result->block);
                }
            });
        }

        std::shared_ptr<spdlog::logger> RippleLikeBlockchainObserver::logger() const {
            return RippleBlockchainObserver::logger();
        }
    }
}