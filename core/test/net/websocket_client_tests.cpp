/*
 *
 * websocket_client_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/10/2017.
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

#include <gtest/gtest.h>
#include <net/QtWebSocketClient.h>
#include <async/QtThreadDispatcher.hpp>
#include <ledger/core/net/WebSocketClient.h>
#include <QCoreApplication>
#include <ledger/core/utils/Option.hpp>
#include <ledger/core/async/Future.hpp>
#include <net/WebSocketConnection.h>

using namespace ledger::core;
using namespace ledger::qt;

TEST(WebSocket, ConnectWebSocket) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto engine = std::make_shared<QtWebSocketClient>(nullptr);
    auto client = std::make_shared<WebSocketClient>(engine);
    auto worker = dispatcher->getSerialExecutionContext("worker");
    auto handler = [] (WebSocketEventType event, const std::shared_ptr<WebSocketConnection>& connection,
                       const Option<std::string>& message, Option<api::ErrorCode> code) {

    };

    client->connect("wss://echo.websocket.org", handler).onComplete(worker, [&] (const TryPtr<WebSocketConnection>& result) {
        EXPECT_TRUE(result.isSuccess());
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
}

TEST(WebSocket, ConnectToInvalidHost) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto engine = std::make_shared<QtWebSocketClient>(nullptr);
    auto client = std::make_shared<WebSocketClient>(engine);
    auto worker = dispatcher->getSerialExecutionContext("worker");
    auto handler = [] (WebSocketEventType event, const std::shared_ptr<WebSocketConnection>& connection,
                       const Option<std::string>& message, Option<api::ErrorCode> code) {

    };

    client->connect("wss://echo.websocket.org/does/not/exist", handler).onComplete(worker, [&] (const TryPtr<WebSocketConnection>& result) {
        EXPECT_TRUE(result.isFailure());
        dispatcher->stop();
    });

    dispatcher->waitUntilStopped();
}

TEST(WebSocket, EchoAndDisconnect) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto engine = std::make_shared<QtWebSocketClient>();
    auto client = std::make_shared<WebSocketClient>(engine);
    auto worker = dispatcher->getSerialExecutionContext("worker");
    auto main = dispatcher->getMainExecutionContext();
    bool hasReceived = false;
    auto handler = [&] (WebSocketEventType event, const std::shared_ptr<WebSocketConnection>& connection,
                       const Option<std::string>& message, Option<api::ErrorCode> code) mutable {
        switch (event) {
            case WebSocketEventType::CONNECT:break;
            case WebSocketEventType::RECEIVE:
                EXPECT_EQ("echo from Ledger", message.getValue());
                hasReceived = true;
                connection->close();
                break;
            case WebSocketEventType::CLOSE:
                dispatcher->stop();
                break;
        }
    };

    client->connect("wss://echo.websocket.org", handler).onComplete(main, [&] (const TryPtr<WebSocketConnection>& result) {
        EXPECT_TRUE(result.isSuccess());
        if (result.isSuccess()) {
            result.getValue()->send("echo from Ledger");
        }
    });

    dispatcher->waitUntilStopped();
    EXPECT_TRUE(hasReceived);
}