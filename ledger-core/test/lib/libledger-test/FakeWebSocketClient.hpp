/*
 *
 * FakeWebSocketClient.h
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

#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <memory>

#include <core/api/ErrorCode.hpp>
#include <core/api/WebSocketClient.hpp>

class FakeWebSocketClient : public ledger::core::api::WebSocketClient {
public:
    FakeWebSocketClient();
    void
    connect(const std::string &url, const std::shared_ptr<ledger::core::api::WebSocketConnection> &connection) override;
    void
    send(const std::shared_ptr<ledger::core::api::WebSocketConnection> &connection, const std::string &data) override;
    void disconnect(const std::shared_ptr<ledger::core::api::WebSocketConnection> &connection) override;
    void push(const std::string& message);
    void closeAll(ledger::core::api::ErrorCode code, std::string& message);
    void setOnConnectCallback(std::function<void ()> f);
    void setOnCloseCallback(std::function<void ()> f);

private:
    std::list<std::shared_ptr<ledger::core::api::WebSocketConnection>> _connections;
    std::atomic<int> _id;
    std::function<void ()> _closeCallback;
    std::function<void ()> _connectCallback;

};
