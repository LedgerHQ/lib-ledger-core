/*
 *
 * FakeWebSocketClient.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 11/10/2017.
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

#include "FakeWebSocketClient.hpp"
#include <api/WebSocketConnection.hpp>

FakeWebSocketClient::FakeWebSocketClient() : _id(0) {
    _closeCallback = [] () {};
    _connectCallback = [] () {};
}

void FakeWebSocketClient::connect(const std::string &url,
                                  const std::shared_ptr<ledger::core::api::WebSocketConnection> &connection) {
    connection->onConnect(_id.fetch_add(1));
    _connections.push_back(connection);
    _connectCallback();
}

void FakeWebSocketClient::send(const std::shared_ptr<ledger::core::api::WebSocketConnection> &connection,
                               const std::string &data) {

}

void FakeWebSocketClient::disconnect(const std::shared_ptr<ledger::core::api::WebSocketConnection> &connection) {
    connection->onClose();
    _closeCallback();
}

void FakeWebSocketClient::push(const std::string& message) {
    for (auto& connection : _connections) {
        connection->onMessage(message);
    }
}

void FakeWebSocketClient::setOnConnectCallback(std::function<void()> f) {
    _connectCallback = f;
}

void FakeWebSocketClient::setOnCloseCallback(std::function<void()> f) {
    _closeCallback = f;
}

void FakeWebSocketClient::closeAll(ledger::core::api::ErrorCode code, std::string &message) {
    for (auto& connection : _connections) {
        connection->onError(code, message);
        disconnect(connection);
    }
}
