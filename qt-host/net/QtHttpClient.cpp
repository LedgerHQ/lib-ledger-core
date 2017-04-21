/*
 *
 * QtHttpClient
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/04/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "QtHttpClient.hpp"

namespace ledger {
    namespace qt {

        class UrlConnection : public core::api::HttpUrlConnection {
        public:
            UrlConnection(int32_t statusCode,
                          std::string statusText,
                          std::unordered_map<std::string, std::string> headers,
                          QByteArray body
            ) {
                _statusCode = statusCode;
                _statusText = statusText;
                _body = body;
                _headers = headers;
            }

            int32_t getStatusCode() override {
                return _statusCode;
            }

            std::string getStatusText() override {
                return _statusText;
            }

            std::unordered_map<std::string, std::string> getHeaders() override {
                return _headers;
            }

            core::api::HttpReadBodyResult readBody() override {
                std::vector<uint8_t> body((uint8_t *)_body.data(), (uint8_t *)(_body.data() + _body.size()));
                _body.clear();
                return core::api::HttpReadBodyResult(
                   std::experimental::optional<core::api::Error>(),
                   std::experimental::optional<std::vector<uint8_t>>(body)
                );
            }

        private:
            int32_t _statusCode;
            std::string _statusText;
            std::unordered_map<std::string, std::string> _headers;
            QByteArray _body;
        };


        void QtHttpClient::execute(const std::shared_ptr<core::api::HttpRequest> &request) {
            QUrl url(QString::fromStdString(request->getUrl()));
            QNetworkRequest r(url);
            // Fill HTTP headers

            for (auto& header : request->getHeaders()) {
                QByteArray key(header.first.c_str(), (int) header.first.length());
                QByteArray value(header.second.c_str(), (int) header.second.length());
                r.setRawHeader(key, value);
            }
            QNetworkReply *reply;

            switch (request->getMethod()) {
                case core::api::HttpMethod::POST: {
                    auto body = request->getBody();
                    reply = _manager.put(r, QByteArray((char *) body.data(), (int) body.size()));
                    break;
                }
                case core::api::HttpMethod::GET:
                    reply = _manager.get(r);
                    break;
                case core::api::HttpMethod::PUT: {
                    auto body = request->getBody();
                    reply = _manager.put(r, QByteArray((char *) body.data(), (int) body.size()));
                    break;
                }
                case core::api::HttpMethod::DEL:
                    reply = _manager.deleteResource(r);
                    break;
            }

            auto callback = [request, reply] () {
                auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                auto statusText = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);


                if (reply->error() == QNetworkReply::NoError || statusCode.isValid()) {
                    std::unordered_map<std::string, std::string> headers;

                    auto body = reply->readAll();
                    auto connection = std::make_shared<UrlConnection>(
                    (statusCode.isValid() ? statusCode.toInt() : 0),
                    (statusText.isValid() ? statusText.toString().toStdString() : reply->errorString().toStdString()),
                        headers,
                        body
                    );

                    request->complete(connection, std::experimental::optional<core::api::Error>());
                } else {
                    core::api::ErrorCode code = core::api::ErrorCode::UNKNOWN;
                    switch (reply->error()) {
                        case QNetworkReply::NoError:break;
                        case QNetworkReply::ConnectionRefusedError:
                            code = core::api::ErrorCode::UNABLE_TO_CONNECT_TO_HOST;
                            break;
                        case QNetworkReply::RemoteHostClosedError:
                            code = core::api::ErrorCode::UNABLE_TO_CONNECT_TO_HOST;
                            break;
                        case QNetworkReply::HostNotFoundError:
                            code = core::api::ErrorCode::UNABLE_TO_RESOLVE_HOST;
                            break;
                        case QNetworkReply::TimeoutError:

                            break;
                        case QNetworkReply::OperationCanceledError:
                            code = core::api::ErrorCode::SSL_ERROR;
                            break;
                        case QNetworkReply::SslHandshakeFailedError:
                            code = core::api::ErrorCode::SSL_ERROR;
                            break;
                        case QNetworkReply::TemporaryNetworkFailureError:
                            code = core::api::ErrorCode::NO_INTERNET_CONNECTIVITY;
                            break;
                        case QNetworkReply::NetworkSessionFailedError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::BackgroundRequestNotAllowedError:break;
                        case QNetworkReply::TooManyRedirectsError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::InsecureRedirectError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::UnknownNetworkError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::ProxyConnectionRefusedError:
                            code = core::api::ErrorCode::PROXY_ERROR;
                            break;
                        case QNetworkReply::ProxyConnectionClosedError:
                            code = core::api::ErrorCode::PROXY_ERROR;
                            break;
                        case QNetworkReply::ProxyNotFoundError:
                            code = core::api::ErrorCode::PROXY_ERROR;
                            break;
                        case QNetworkReply::ProxyTimeoutError:
                            code = core::api::ErrorCode::PROXY_ERROR;
                            break;
                        case QNetworkReply::ProxyAuthenticationRequiredError:
                            code = core::api::ErrorCode::PROXY_ERROR;
                            break;
                        case QNetworkReply::UnknownProxyError:
                            code = core::api::ErrorCode::PROXY_ERROR;
                            break;
                        case QNetworkReply::ContentAccessDenied:break;
                        case QNetworkReply::ContentOperationNotPermittedError:
                            code = core::api::ErrorCode::AUTHENTICATION_REQUIRED;
                            break;
                        case QNetworkReply::ContentNotFoundError:break;
                        case QNetworkReply::AuthenticationRequiredError:
                            code = core::api::ErrorCode::AUTHENTICATION_REQUIRED;
                            break;
                        case QNetworkReply::ContentReSendError:break;
                        case QNetworkReply::ContentConflictError:break;
                        case QNetworkReply::ContentGoneError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::UnknownContentError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::ProtocolUnknownError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::ProtocolInvalidOperationError:break;
                        case QNetworkReply::ProtocolFailure:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                        case QNetworkReply::InternalServerError:break;
                        case QNetworkReply::OperationNotImplementedError:break;
                        case QNetworkReply::ServiceUnavailableError:break;
                        case QNetworkReply::UnknownServerError:
                            code = core::api::ErrorCode::HTTP_ERROR;
                            break;
                    }
                    request->complete(nullptr, std::experimental::optional<core::api::Error>(core::api::Error(
                        code, reply->errorString().toStdString()
                    )));
                }

            };

            connect(reply, &QNetworkReply::finished, [callback] () -> void {
                callback();
            });

        }
    }
}