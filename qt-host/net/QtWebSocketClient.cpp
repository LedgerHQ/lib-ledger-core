/*
 *
 * QtWebSocketClient.cpp
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

#include "QtWebSocketClient.h"
#include "../../cmake-build-debug/include/ledger/core/api/ErrorCode.hpp"

using namespace ledger::core;

namespace ledger {
    namespace qt {

        QtWebSocketClient::QtWebSocketClient(QObject *parent) : QObject(parent) {

        }

        void QtWebSocketClient::connect(const std::string &url,
                                        const std::shared_ptr<core::api::WebSocketConnection> &connection) {
            if (getConnection(connection)) {
                connection->onError(api::ErrorCode::ILLEGAL_STATE, "This connection is already in use");
            } else {
                auto conn = std::make_shared<Connection>();
                conn->api = connection;
                _connections.push_back(conn);
                QObject::connect(&conn->socket, &QWebSocket::connected, this, &QtWebSocketClient::onWebSocketConnected);
                QObject::connect(&conn->socket, &QWebSocket::disconnected, this, &QtWebSocketClient::onWebSocketDisconnected);
                QObject::connect(&conn->socket, &QWebSocket::textMessageReceived, this, &QtWebSocketClient::onMessageReceived);
                QObject::connect(&conn->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                                 this, SLOT(onError(QAbstractSocket::SocketError)));
                conn->socket.open(QUrl(QString::fromStdString(url)));
            }
        }

        void QtWebSocketClient::send(const std::shared_ptr<core::api::WebSocketConnection> &connection,
                                     const std::string &data) {
            auto conn = getConnection(connection);
            if (!conn) {
                connection->onError(api::ErrorCode::ILLEGAL_STATE, "Connection doesn't exist");
            } else if (conn->api->getConnectionId() == -1) {
                connection->onError(api::ErrorCode::ILLEGAL_STATE, "Connection is not ready");
            } else {
                conn->socket.sendTextMessage(QString::fromStdString(data));
            }

        }

        void QtWebSocketClient::disconnect(const std::shared_ptr<core::api::WebSocketConnection> &connection) {
            auto conn = getConnection(connection);
            if (conn != nullptr && conn->api != nullptr) {
                conn->socket.close(QWebSocketProtocol::CloseCode::CloseCodeNormal, "Disconnected by user");
            }
        }

        std::shared_ptr<QtWebSocketClient::Connection> QtWebSocketClient::getConnection(QObject *sender) const {
            for (auto& conn : _connections) {
                if (static_cast<QObject *>(&conn->socket) == sender)
                    return conn;
            }
            return nullptr;
        }

        std::shared_ptr<QtWebSocketClient::Connection>
        QtWebSocketClient::getConnection(const std::shared_ptr<core::api::WebSocketConnection> &connection) const {
            for (auto& conn : _connections) {
                if (conn->api == connection)
                    return conn;
            }
            return nullptr;
        }

        bool QtWebSocketClient::close(QtWebSocketClient::Connection *connection) {
            auto it = _connections.begin();
            auto end = _connections.end();
            while (it != end) {
                auto conn = *it;
                if (conn.get() == connection) {
                    QObject::disconnect(&conn->socket, &QWebSocket::connected, this, &QtWebSocketClient::onWebSocketConnected);
                    QObject::disconnect(&conn->socket, &QWebSocket::disconnected, this, &QtWebSocketClient::onWebSocketDisconnected);
                    QObject::disconnect(&conn->socket, &QWebSocket::textMessageReceived, this, &QtWebSocketClient::onMessageReceived);
                    QObject::disconnect(&conn->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                                     this, SLOT(onError(QAbstractSocket::SocketError)));
                    _connections.erase(it);
                    return true;
                }
                it++;
            }
            return false;
        }

        void QtWebSocketClient::onWebSocketConnected() {
            auto conn = getConnection(QObject::sender());
            if (conn != nullptr) {
                conn->api->onConnect(_lastId.fetchAndAddAcquire(1));
            }
        }

        void QtWebSocketClient::onWebSocketDisconnected() {
            auto conn = getConnection(QObject::sender());
            if (conn != nullptr) {
                conn->api->onClose();
            }
        }

        void QtWebSocketClient::onMessageReceived(QString message) {
            auto conn = getConnection(QObject::sender());
            if (conn != nullptr) {
                conn->api->onMessage(message.toStdString());
            }
        }

        void QtWebSocketClient::onError(QAbstractSocket::SocketError error) {
            auto conn = getConnection(QObject::sender());
            if (conn != nullptr) {
                QString message;
                switch (error) {
                    case QAbstractSocket::ConnectionRefusedError:
                        message = QString("Connection refused to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::RemoteHostClosedError:
                        message = QString("Remote host closed connection to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::HostNotFoundError:
                        message = QString("Host not found for %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::SocketAccessError:
                        message = QString("Socket access error to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::SocketResourceError:
                        message = QString("Socket resource failed to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::SocketTimeoutError:
                        message = QString("Socket timed out to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::DatagramTooLargeError:
                        message = QString("Too large datagram for %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::NetworkError:
                        message = QString("Network error to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::AddressInUseError:
                        message = QString("Address is in use for %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::SocketAddressNotAvailableError:
                        message = QString("Address %1 not available").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::UnsupportedSocketOperationError:
                        message = QString("Unsopported operation to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::UnfinishedSocketOperationError:
                        message = QString("Unfinished operation for %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::ProxyAuthenticationRequiredError:
                        message = QString("Proxy authentication required");
                        break;
                    case QAbstractSocket::SslHandshakeFailedError:
                        message = QString("SSL handshake failed to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::ProxyConnectionRefusedError:
                        message = QString("Proxy connection refused to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::ProxyConnectionClosedError:
                        message = QString("Proxy connection closed to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::ProxyConnectionTimeoutError:
                        message = QString("Proxy connection timed out to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::ProxyNotFoundError:
                        message = QString("Proxy not found");
                        break;
                    case QAbstractSocket::ProxyProtocolError:
                        message = QString("Proxy protocol error");
                        break;
                    case QAbstractSocket::OperationError:
                        message = QString("Operation error to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::SslInternalError:
                        message = QString("SSL internal error to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::SslInvalidUserDataError:
                        message = QString("SSSL invalid user data error for %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::TemporaryError:
                        message = QString("Tempory error to %1").arg(conn->socket.requestUrl().toString());
                        break;
                    case QAbstractSocket::UnknownSocketError:
                        message = QString("Connection refused to %1").arg(conn->socket.requestUrl().toString());
                        break;
                }
                conn->api->onError(api::ErrorCode::HTTP_ERROR, message.toStdString());
                close(conn.get());
            }
        }
    }
}