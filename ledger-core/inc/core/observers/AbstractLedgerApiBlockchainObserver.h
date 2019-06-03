/*
 *
 * AbstractLedgerApiBlockchainObserver
 *
 * Created by El Khalil Bellakrid on 29/11/2018.
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

#include <core/observers/AbstractBlockchainObserver.h>
#include <core/net/WebSocketClient.h>
#include <core/net/WebSocketConnection.h>

namespace ledger {
    namespace core {
        class AbstractLedgerApiBlockchainObserver {
        private:
            virtual std::shared_ptr<spdlog::logger> logger() const = 0;
            virtual void connect() = 0;
            virtual void reconnect() = 0;

            virtual void onMessage(const std::string& message) = 0;

        protected:
            void onSocketEvent(WebSocketEventType event,
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
            };
            std::shared_ptr<WebSocketConnection> _socket;
            int32_t _attempt;
            std::string _url;
        };
    }
}
