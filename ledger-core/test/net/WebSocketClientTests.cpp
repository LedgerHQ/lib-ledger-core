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
#include <gmock/gmock.h>
#include <QCoreApplication>

#include <core/async/Future.hpp>
#include <core/net/WebSocketClient.hpp>
#include <core/net/WebSocketConnection.hpp>
#include <core/utils/Option.hpp>

#include <async/QtThreadDispatcher.hpp>

using namespace ledger::core;
using namespace ledger::qt;
using namespace testing;

class MockApiWebSocketClient : public api::WebSocketClient {
public:
    MOCK_METHOD2(connect, void (const std::string & url, const std::shared_ptr<api::WebSocketConnection> & connection));
    MOCK_METHOD2(send, void (const std::shared_ptr<api::WebSocketConnection> & connection, const std::string & data));
    MOCK_METHOD1(disconnect, void (const std::shared_ptr<api::WebSocketConnection> & connection));
};

class MockHandler {
public:
    MOCK_METHOD4(Handle, void (WebSocketEventType,
                            const std::shared_ptr<WebSocketConnection>& connection,
                            const Option<std::string>& message,
                            Option<api::ErrorCode>));
};

class WebSocketClientTest : public Test {
public:
    WebSocketClientTest()
        : engine(std::make_shared<MockApiWebSocketClient>())
        , handler([&h = mockHandler](WebSocketEventType a,
                                    const std::shared_ptr<WebSocketConnection>& b,
                                    const Option<std::string>& c,
                                    Option<api::ErrorCode> d) { h.Handle(a,b,c,d); }) {};
public:
    void TearDown() override {
        // there are circular dependencies between WebSocketClient and WebSocketConnection
        // so without connection.close() there is a leak
        Mock::AllowLeak(engine.get());
    }

    const std::string url = "uri://some_address";
    const std::string message = "Hello from websocket";
    MockHandler mockHandler;
    std::shared_ptr<MockApiWebSocketClient> engine;
    WebSocketEventHandler handler;
    const int32_t connectionID = 42;
};

TEST_F(WebSocketClientTest, InjectionIsDone) {
    //Injection happened once
    EXPECT_CALL(*engine, connect(StrEq(url), _)).Times(1);
    // callback was not called
    EXPECT_CALL(mockHandler, Handle(_, _, _, _)).Times(0);

    WebSocketClient client(engine);
    auto connectionFtr = client.connect(url, handler);
    EXPECT_FALSE(connectionFtr.isCompleted());
}

TEST_F(WebSocketClientTest, ConnectSuccedsSync) {
    EXPECT_CALL(*engine, connect(_, _))
        .WillOnce(Invoke([connID = connectionID](Unused, const std::shared_ptr<api::WebSocketConnection>& connImpl) {
            connImpl->onConnect(connID);
        }));
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CONNECT,
                                    _,
                                    Option<std::string>::NONE,
                                    Option<api::ErrorCode>::NONE)).Times(1);
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CLOSE, _, _, _)).Times(0);

    WebSocketClient client(engine);
    auto connectionFtr = client.connect(url, handler);
    ASSERT_TRUE(connectionFtr.isCompleted());
    auto connectionTry = connectionFtr.getValue().getValue();
    ASSERT_TRUE(connectionTry.isSuccess());
    EXPECT_EQ(connectionTry.getValue()->getApiConnection()->getConnectionId(), connectionID);
}

TEST_F(WebSocketClientTest, ConnectFailed) {
    const std::string errorMessge("Can't connect");
    api::ErrorCode errorCode = api::ErrorCode::UNABLE_TO_CONNECT_TO_HOST;
    EXPECT_CALL(*engine, connect(_, _))
        .WillOnce(Invoke([errorCode, errorMessge](Unused, const std::shared_ptr<api::WebSocketConnection>& connImpl) {
            connImpl->onError(errorCode, errorMessge);
        }));
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CONNECT, _, _, _)).Times(0);
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CLOSE,
                                    _,
                                    _,
                                    Option<api::ErrorCode>(errorCode))).Times(1);

    WebSocketClient client(engine);
    auto connectionFtr = client.connect(url, handler);
    ASSERT_TRUE(connectionFtr.isCompleted());
    auto connectionTry = connectionFtr.getValue().getValue();
    ASSERT_TRUE(connectionTry.isFailure());
    EXPECT_EQ(connectionTry.getFailure().what(), errorMessge);
    EXPECT_EQ(connectionTry.getFailure().getErrorCode(), errorCode);
}

TEST_F(WebSocketClientTest, SendOK) {
    EXPECT_CALL(*engine, connect(_, _))
        .WillOnce(Invoke([connID = connectionID](Unused, const std::shared_ptr<api::WebSocketConnection>& connImpl) {
            connImpl->onConnect(connID);
        }));
    EXPECT_CALL(*engine, send(_, StrEq(message))).Times(1);
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CONNECT,
                                    _,
                                    Option<std::string>::NONE,
                                    Option<api::ErrorCode>::NONE)).Times(1);
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CLOSE, _, _, _)).Times(0);

    WebSocketClient client(engine);
    auto connection = client.connect(url, handler).getValue().getValue().getValue();
    connection->send(message);
}

TEST_F(WebSocketClientTest, SendError) {
    const std::string errorMessage("Can't reach the host");
    api::ErrorCode errorCode = api::ErrorCode::NO_INTERNET_CONNECTIVITY;
    EXPECT_CALL(*engine, connect(_, _))
        .WillOnce(Invoke([connID = connectionID](Unused, const std::shared_ptr<api::WebSocketConnection>& connImpl) {
            connImpl->onConnect(connID);
        }));
    EXPECT_CALL(*engine, send(_, StrEq(message))).
        WillOnce(Invoke([errorCode, errorMessage](const std::shared_ptr<api::WebSocketConnection>& connImpl, const std::string& msg) {
            connImpl->onError(errorCode, errorMessage);
        }));
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CONNECT,
                                    _,
                                    Option<std::string>::NONE,
                                    Option<api::ErrorCode>::NONE)).Times(1);
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CLOSE, _, _, _)).Times(1);

    WebSocketClient client(engine);
    auto connection = client.connect(url, handler).getValue().getValue().getValue();
    connection->send(message);
}

TEST_F(WebSocketClientTest, EchoTest) {
    EXPECT_CALL(*engine, connect(_, _))
        .WillOnce(Invoke([connID = connectionID](Unused, const std::shared_ptr<api::WebSocketConnection>& connImpl) {
            connImpl->onConnect(connID);
        }));
    EXPECT_CALL(*engine, send(_, StrEq(message))).
        WillOnce(Invoke([](const std::shared_ptr<api::WebSocketConnection>& connImpl, const std::string& msg) {
            connImpl->onMessage(msg);
        }));
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CONNECT,
                                    _,
                                    Option<std::string>::NONE,
                                    Option<api::ErrorCode>::NONE)).Times(1);
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::CLOSE, _, _, _)).Times(0);
    EXPECT_CALL(mockHandler, Handle(WebSocketEventType::RECEIVE, _, Option<std::string>(message), Option<api::ErrorCode>::NONE)).Times(1);

    WebSocketClient client(engine);
    auto connection = client.connect(url, handler).getValue().getValue().getValue();
    connection->send(message);
}
