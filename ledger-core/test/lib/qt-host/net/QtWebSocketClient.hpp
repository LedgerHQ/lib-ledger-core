/*
 *
 * QtWebSocketClient.h
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

#pragma once

#include <QAtomicInt>
#include <QHash>
#include <QtWebSockets/QtWebSockets>

#include <core/api/WebSocketClient.hpp>
#include <core/api/WebSocketConnection.hpp>

namespace ledger {
    namespace qt {
        class QtWebSocketClient : public QObject, public core::api::WebSocketClient {
            Q_OBJECT
        public:
            explicit QtWebSocketClient(QObject* parent = Q_NULLPTR);
            void
            connect(const std::string &url, const std::shared_ptr<core::api::WebSocketConnection> &connection) override;
            void
            send(const std::shared_ptr<core::api::WebSocketConnection> &connection, const std::string &data) override;
            void disconnect(const std::shared_ptr<core::api::WebSocketConnection> &connection) override;

        private Q_SLOTS:
            void onWebSocketConnected();
            void onWebSocketDisconnected();
            void onMessageReceived(QString message);
            void onError(QAbstractSocket::SocketError error);

        private:
            struct Connection {
                QWebSocket socket;
                std::shared_ptr<core::api::WebSocketConnection> api;
                ~Connection() = default;
            };
            std::shared_ptr<Connection> getConnection(QObject* sender) const;
            std::shared_ptr<Connection> getConnection(const std::shared_ptr<core::api::WebSocketConnection>& conn) const;
            bool close(Connection *connection);

            std::vector<std::shared_ptr<Connection>> _connections;
            QAtomicInt _lastId;
        };
    }
}
