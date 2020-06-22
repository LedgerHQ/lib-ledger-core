/*
 *
 * CosmosLikeBlockchainObserver
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <core/api/Configuration.hpp>
#include <core/api/ConfigurationDefaults.hpp>
#include <core/api/CosmosConfigurationDefaults.hpp>
#include <core/math/Fibonacci.hpp>
#include <core/utils/JSONUtils.hpp>
#include <core/wallet/Block.hpp>
#include <cosmos/Account.hpp>
#include <cosmos/observers/BlockchainObserver.hpp>

namespace ledger {
namespace core {
namespace cosmos {
CosmosLikeBlockchainObserver::CosmosLikeBlockchainObserver(
    const std::shared_ptr<api::ExecutionContext> &context,
    const std::shared_ptr<api::DynamicObject> &configuration,
    const std::shared_ptr<spdlog::logger> &logger,
    const api::Currency &currency,
    const std::vector<std::string> &matchableKeys)
    : DedicatedContext(context), ConfigurationMatchable(matchableKeys),
      _currency(currency), _configuration(configuration) {
  setConfiguration(configuration);
  setLogger(logger);
}

CosmosLikeBlockchainObserver::CosmosLikeBlockchainObserver(
    const std::shared_ptr<api::ExecutionContext> &context,
    const std::shared_ptr<WebSocketClient> &client,
    const std::shared_ptr<api::DynamicObject> &configuration,
    const std::shared_ptr<spdlog::logger> &logger,
    const api::Currency &currency)
    : CosmosLikeBlockchainObserver(
          context, configuration, logger, currency,
          {api::CosmosConfigurationDefaults::COSMOS_OBSERVER_WS_ENDPOINT}) {
  _client = client;
  _url = getConfiguration()
             ->getString(api::Configuration::BLOCKCHAIN_OBSERVER_WS_ENDPOINT)
             .value_or(
                 api::CosmosConfigurationDefaults::COSMOS_OBSERVER_WS_ENDPOINT);
}

void CosmosLikeBlockchainObserver::putTransaction(
    const cosmos::Transaction &tx) {
  std::lock_guard<std::mutex> lock(_lock);
  for (const auto &account : _accounts) {
    account->run([account, tx]() {
      soci::session sql(account->getWallet()->getDatabase()->getPool());
      if (account->putTransaction(sql, tx) !=
          CosmosLikeAccount::FLAG_TRANSACTION_IGNORED)
        account->emitEventsNow();
    });
  }
}

void CosmosLikeBlockchainObserver::putBlock(const cosmos::Block &block) {
  std::lock_guard<std::mutex> lock(_lock);
  for (const auto &account : _accounts) {
    account->run([account, block]() {
      bool shouldEmitNow = false;
      {
        soci::session sql(account->getWallet()->getDatabase()->getPool());
        shouldEmitNow = account->putBlock(sql, block.toApiBlock());
      }
      if (shouldEmitNow)
        account->emitEventsNow();
    });
  }
}

void CosmosLikeBlockchainObserver::onStart() { connect(); }

void CosmosLikeBlockchainObserver::onStop() {
  auto self = shared_from_this();
  run([self]() {
    if (self->_socket != nullptr)
      self->_socket->close();
    self->_handler =
        [self](WebSocketEventType event,
               const std::shared_ptr<WebSocketConnection> &connection,
               const Option<std::string> &message,
               Option<api::ErrorCode> code) {

        };
  });
}

void CosmosLikeBlockchainObserver::connect() {
  CosmosBlockchainObserver::logger()->info("Connect {} observer",
                                           getCurrency().name);
  auto self = shared_from_this();
  _handler = [self](WebSocketEventType event,
                    const std::shared_ptr<WebSocketConnection> &connection,
                    const Option<std::string> &message,
                    Option<api::ErrorCode> code) {
    self->onSocketEvent(event, connection, message, code);
  };
  _client->connect(_url, _handler);
}

void CosmosLikeBlockchainObserver::reconnect() {
  auto self = shared_from_this();
  auto delay =
      std::min(std::max(Fibonacci::compute(_attempt) * 500, 30000), 500);
  CosmosBlockchainObserver::logger()->info(
      "Attempt reconnection in {}ms for {} observer", delay,
      getCurrency().name);
  getContext()->delay(LambdaRunnable::make([self]() { self->connect(); }),
                      delay);
}

void CosmosLikeBlockchainObserver::onMessage(const std::string &message) {
  auto self = shared_from_this();
  run([message, self]() {
    //                auto result =
    //                JSONUtils::parse<CosmosLikeWebSocketNotificationParser>(message);
    //                result->block.currencyName = self->getCurrency().name;
    throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                         "Not implemented");
  });
}

std::shared_ptr<spdlog::logger> CosmosLikeBlockchainObserver::logger() const {
  return CosmosBlockchainObserver::logger();
}
} // namespace cosmos
} // namespace core
} // namespace ledger
