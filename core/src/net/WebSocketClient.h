/*
 *
 * WebSocketClient.h
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

#ifndef LEDGER_CORE_WEBSOCKETCLIENT_H
#define LEDGER_CORE_WEBSOCKETCLIENT_H

#include <api/WebSocketClient.hpp>
#include <api/WebSocketConnection.hpp>
#include <async/Future.hpp>

namespace ledger {
    namespace core {
        class WebSocketConnection;
        enum class WebSocketEventType {
            CONNECT,
            RECEIVE,
            CLOSE
        };
        using WebSocketEventHandler = std::function<void(WebSocketEventType,
                                                         const std::shared_ptr<WebSocketConnection> &connection,
                                                         const Option<std::string> &message,
                                                         Option<api::ErrorCode>)>;
        class WebSocketClient {
          public:
            WebSocketClient(const std::shared_ptr<api::WebSocketClient> &client);
            FuturePtr<WebSocketConnection> connect(const std::string &url, const WebSocketEventHandler &handler);

          private:
            std::shared_ptr<api::WebSocketClient> _client;
        };
    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_WEBSOCKETCLIENT_H
