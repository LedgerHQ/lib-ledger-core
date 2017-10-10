/*
 *
 * WebSocketConnection.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 06/10/2017.
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

#include "WebSocketConnection.h"

namespace ledger {
    namespace core {
        class WebSocketConnImpl : public api::WebSocketConnection {
        public:
            WebSocketConnImpl(const WebSocketEventHandler& handler, const std::shared_ptr<ledger::core::WebSocketConnection>& conn) {
                _conn = conn;
                _handler = handler;
            }

            void onConnect(int32_t connectionId) override {
                _id = connectionId;
                _handler(WebSocketEventType::CONNECT, _conn, Option<std::string>(), Option<api::ErrorCode>());
            }

            void onClose() override {
                _handler(WebSocketEventType::CLOSE, _conn, Option<std::string>(), Option<api::ErrorCode>());
                _conn = nullptr;
            }

            void onMessage(const std::string &data) override {
                if (_conn)
                    _handler(WebSocketEventType::RECEIVE, _conn, Option<std::string>(data), Option<api::ErrorCode>());
            }

            void onError(api::ErrorCode code, const std::string &message) override {
                if (_conn)
                    _handler(WebSocketEventType::CLOSE, _conn, Option<std::string>(message), Option<api::ErrorCode>(code));
            }

            int32_t getConnectionId() override {
                return _id;
            }

        private:
            int32_t _id;
            WebSocketEventHandler _handler;
            std::shared_ptr<ledger::core::WebSocketConnection> _conn;
        };

        WebSocketConnection::WebSocketConnection(const std::shared_ptr<api::WebSocketClient> &client,
                                                 const WebSocketEventHandler &handler) {
            _client = client;
            _handler = handler;
        }

        std::shared_ptr<api::WebSocketConnection> WebSocketConnection::getApiConnection() {
            if (!_api)
                _api = std::make_shared<WebSocketConnImpl>(_handler, shared_from_this());
            return _api;
        }

        void WebSocketConnection::send(const std::string &message) {
            _client->send(_api, message);
        }

        void WebSocketConnection::close() {
            if (_api) {
                _client->disconnect(_api);
                _api = nullptr;
            }
        }
    }
}