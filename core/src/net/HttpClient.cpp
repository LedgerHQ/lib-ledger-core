/*
 *
 * HttpClient
 * ledger-core
 *
 * Created by Pierre Pollastri on 30/11/2016.
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
#include "HttpClient.hpp"

namespace ledger {
    namespace core {

        HttpClient::HttpClient(const std::string &baseUrl, const std::shared_ptr<api::HttpClient> &client,
                               const std::shared_ptr<api::ExecutionContext> &context) {
            _baseUrl = baseUrl;
            _client = client;
            _context = context;
            if (_baseUrl.back() != '/') {
                _baseUrl += "/";
            }
        }

        HttpRequest HttpClient::GET(const std::string &path,
                                    const std::unordered_map<std::string, std::string> &headers,
                                    const std::string &baseUrl) {
            return createRequest(api::HttpMethod::GET, path, std::experimental::optional<std::vector<uint8_t>>(), headers, baseUrl);
        }

        HttpRequest HttpClient::PUT(const std::string &path, const std::vector<uint8_t> &body,
                                    const std::unordered_map<std::string, std::string> &headers) {
            return createRequest(api::HttpMethod::PUT, path, std::experimental::optional<std::vector<uint8_t>>(), headers);
        }

        HttpRequest HttpClient::DEL(const std::string &path, const std::unordered_map<std::string, std::string> &headers) {
            return createRequest(api::HttpMethod::DEL, path, std::experimental::optional<std::vector<uint8_t>>(), headers);
        }

        HttpRequest HttpClient::POST(const std::string &path,
                                     const std::vector<uint8_t> &body,
                                     const std::unordered_map<std::string, std::string> &headers,
                                     const std::string &baseUrl) {
            return createRequest(api::HttpMethod::POST,
                                 path,
                                 std::experimental::optional<std::vector<uint8_t>>(body),
                                 headers,
                                 baseUrl);
        }

        HttpClient& HttpClient::addHeader(const std::string &key, const std::string &value) {
            _headers[key] = value;
            return *this;
        }

        HttpClient& HttpClient::removeHeader(const std::string &key) {
            _headers.erase(key);
            return *this;
        }

        HttpRequest HttpClient::createRequest(api::HttpMethod method,
                                              const std::string &path,
                                              const std::experimental::optional<std::vector<uint8_t >> body,
                                              const std::unordered_map<std::string, std::string> &headers,
                                              const std::string &baseUrl) {
            auto url = baseUrl.empty() ? _baseUrl :
                       baseUrl.back() != '/' ? baseUrl + "/" : baseUrl;
            if (path.front() == '/') {
                url += std::string(path.data() + 1);
            } else {
                url += path;
            }
            auto fullheaders = _headers;
            for (const auto& item : headers) {
                fullheaders[item.first] = item.second;
            }
            return HttpRequest(
                    method,
                    url,
                    fullheaders,
                    body,
                    _client,
                    _context,
                    _logger
            );
        }

        void HttpClient::setLogger(const std::shared_ptr<spdlog::logger> &logger) {
            _logger = make_option(logger);
        }

        HttpRequest::HttpRequest(api::HttpMethod method, const std::string &url,
                                 const std::unordered_map<std::string, std::string> &headers,
                                 const std::experimental::optional<std::vector<uint8_t >> body,
                                 const std::shared_ptr<api::HttpClient> &client,
                                 const std::shared_ptr<api::ExecutionContext> &context,
                                 const Option<std::shared_ptr<spdlog::logger>>& logger) {
            _method = method;
            _url = url;
            _headers = headers;
            _body = body;
            _client = client;
            _context = context;
            _logger = logger;
        }

        HttpRequest::ApiRequest::ApiRequest(const std::shared_ptr<const ledger::core::HttpRequest>& self) {
            _self = self;
        }

        std::shared_ptr<api::HttpRequest> HttpRequest::toApiRequest() const {
            auto request = std::shared_ptr<HttpRequest::ApiRequest>(new ApiRequest(std::make_shared<HttpRequest>(*this)));
            return std::dynamic_pointer_cast<api::HttpRequest>(request);
        }

        Future<std::shared_ptr<api::HttpUrlConnection>> HttpRequest::operator()() const {
            auto request = std::dynamic_pointer_cast<ApiRequest>(toApiRequest());
            _client->execute(request);
            _logger.foreach([&] (const std::shared_ptr<spdlog::logger>& logger) {
                logger->info("{} {}", api::to_string(request->getMethod()), request->getUrl());
            });
            auto logger = _logger;
            return  request->getFuture().map<std::shared_ptr<api::HttpUrlConnection>>(_context, [=] (const std::shared_ptr<api::HttpUrlConnection>& connection) {
                logger.foreach([&] (const std::shared_ptr<spdlog::logger>& l) {
                    l->info("{} {} - {} {}", api::to_string(request->getMethod()), request->getUrl(),  connection->getStatusCode(), connection->getStatusText());
                });
                if (connection->getStatusCode() < 200 || connection->getStatusCode() >= 300) {
                    throw Exception(HttpRequest::getErrorCode(connection->getStatusCode()), connection->getStatusText(),
                                    Option<std::shared_ptr<void>>(std::static_pointer_cast<void>(connection)));
                }
                return connection;
            });
        }

        Future<HttpRequest::JsonResult> HttpRequest::json(bool parseNumbersAsString, bool ignoreStatusCode) const {
            return operator()().recover(_context, [] (const Exception& exception) {
                if (HttpRequest::isHttpError(exception.getErrorCode()) &&
                exception.getUserData().nonEmpty()) {
                    return std::static_pointer_cast<api::HttpUrlConnection>(exception.getUserData().getValue());
                }
                throw exception;
            }).map<JsonResult>
                    (_context, [parseNumbersAsString, ignoreStatusCode] (const std::shared_ptr<api::HttpUrlConnection>& co) {
                        std::shared_ptr<api::HttpUrlConnection> connection = co;
                        auto doc = std::make_shared<rapidjson::Document>();
                        HttpUrlConnectionInputStream is(connection);
                        if (parseNumbersAsString) {
                            doc->ParseStream<rapidjson::kParseNumbersAsStringsFlag>(is);
                        } else {
                            doc->ParseStream(is);
                        }
                        std::shared_ptr<JsonResult> result(new std::tuple<std::shared_ptr<api::HttpUrlConnection>, std::shared_ptr<rapidjson::Document>>(std::make_tuple(connection, doc)));
                        if (!ignoreStatusCode && (connection->getStatusCode() < 200 || connection->getStatusCode() >= 300)) {
                            throw Exception(HttpRequest::getErrorCode(connection->getStatusCode()), connection->getStatusText(),
                                            Option<std::shared_ptr<void>>(std::static_pointer_cast<void>(result)));
                        }
                        return std::make_tuple(connection, doc);
            });
        }

        api::HttpMethod HttpRequest::ApiRequest::getMethod() {
            return _self->_method;
        }

        std::unordered_map<std::string, std::string> HttpRequest::ApiRequest::getHeaders() {
            return _self->_headers;
        }

        std::vector<uint8_t> HttpRequest::ApiRequest::getBody() {
            return _self->_body.value_or(std::vector<uint8_t>());
        }

        std::string HttpRequest::ApiRequest::getUrl() {
            return _self->_url;
        }

        HttpRequest::ApiRequest::~ApiRequest() {

        }

        void HttpRequest::ApiRequest::complete(const std::shared_ptr<api::HttpUrlConnection> &response,
                                               const optional<api::Error> &error) {
            if (error) {
                _promise.failure(Exception(error->code, error->message));
            } else {
                _promise.success(response);
            }
        }

        Future<std::shared_ptr<api::HttpUrlConnection>> HttpRequest::ApiRequest::getFuture() const {
            return _promise.getFuture();
        }
    }


}
