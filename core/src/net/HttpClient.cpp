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
#include "../utils/LambdaRunnable.hpp"

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

        HttpRequest HttpClient::GET(const std::string &path, const std::unordered_map<std::string, std::string> &headers) {
            return createRequest(api::HttpMethod::GET, path, std::experimental::optional<std::vector<uint8_t>>(), headers);
        }

        HttpRequest HttpClient::PUT(const std::string &path, const std::vector<uint8_t> &body,
                                    const std::unordered_map<std::string, std::string> &headers) {
            return createRequest(api::HttpMethod::GET, path, std::experimental::optional<std::vector<uint8_t>>(), headers);
        }

        HttpRequest HttpClient::DELETE(const std::string &path, const std::unordered_map<std::string, std::string> &headers) {
            return createRequest(api::HttpMethod::GET, path, std::experimental::optional<std::vector<uint8_t>>(), headers);
        }

        HttpRequest HttpClient::POST(const std::string &path, const std::vector<uint8_t> &body,
                                     const std::unordered_map<std::string, std::string> &headers) {
            return createRequest(api::HttpMethod::GET, path, std::experimental::optional<std::vector<uint8_t>>(), headers);
        }

        void HttpClient::addHeader(const std::string &key, const std::string &value) {
            _headers[key] = value;
        }

        void HttpClient::removeHeader(const std::string &key) {
            _headers.erase(key);
        }

        HttpRequest HttpClient::createRequest(api::HttpMethod method, const std::string &path,
                                              const std::experimental::optional<std::vector<uint8_t >> body,
                                              const std::unordered_map<std::string, std::string> &headers) {
            auto url = _baseUrl;
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
                    _context
            );
        }

        HttpRequest::HttpRequest(api::HttpMethod method, const std::string &url,
                                 const std::unordered_map<std::string, std::string> &headers,
                                 const std::experimental::optional<std::vector<uint8_t >> body,
                                 const std::shared_ptr<api::HttpClient> &client,
                                 const std::shared_ptr<api::ExecutionContext> &context) {
            _method = method;
            _url = url;
            _headers = headers;
            _body = body;
            _client = client;
            _context = context;
        }

        void HttpRequest::operator()(std::function<void (const api::HttpResponse&)> callback) {
            auto req = toApiRequest();
            std::dynamic_pointer_cast<ApiRequest>(req)->setCallback(callback);
            _client->execute(req);
        }

        HttpRequest::ApiRequest::ApiRequest(const  ledger::core::HttpRequest &self) {
            _self = new ledger::core::HttpRequest(self);
        }

        std::shared_ptr<api::HttpRequest> HttpRequest::toApiRequest() const {
            auto request = std::make_shared<HttpRequest::ApiRequest>(*this);
            return std::dynamic_pointer_cast<api::HttpRequest>(request);
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

        void HttpRequest::ApiRequest::complete(const api::HttpResponse &response) {
            auto callback = _callback;
            _self->_context->execute(make_runnable([callback, response] () {
                callback(response);
            }));
        }

        void HttpRequest::ApiRequest::setCallback(std::function<void (const api::HttpResponse&)> callback) {
            _callback = callback;
        }

        HttpRequest::ApiRequest::~ApiRequest() {
            delete _self;
        }
    }


}