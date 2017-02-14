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
#ifndef LEDGER_CORE_HTTPCLIENT_HPP
#define LEDGER_CORE_HTTPCLIENT_HPP

#include "../api/HttpClient.hpp"
#include "../api/HttpMethod.hpp"
#include "../api/HttpRequest.hpp"
#include "../api/HttpUrlConnection.hpp"
#include "../api/HttpReadBodyResult.hpp"
#include "../api/ExecutionContext.hpp"
#include "../utils/optional.hpp"
#include <unordered_map>
#include <memory>
#include "../async/Future.hpp"
#include "../async/Promise.hpp"

namespace ledger {
    namespace core {

        class HttpRequest : public std::enable_shared_from_this<HttpRequest> {
        public:
            HttpRequest(api::HttpMethod method,
                        const std::string& url,
                        const std::unordered_map<std::string, std::string>& headers,
                        const std::experimental::optional<std::vector<uint8_t >> body,
                        const std::shared_ptr<api::HttpClient> &client,
                        const std::shared_ptr<api::ExecutionContext> &context);
            Future<std::shared_ptr<api::HttpUrlConnection>> operator()();
            std::shared_ptr<api::HttpRequest> toApiRequest() const;
        private:
            api::HttpMethod _method;
            std::string _url;
            std::unordered_map<std::string, std::string> _headers;
            std::experimental::optional<std::vector<uint8_t >> _body;
            std::shared_ptr<api::HttpClient> _client;
            std::shared_ptr<api::ExecutionContext> _context;

            class ApiRequest : public api::HttpRequest {
            public:
                ApiRequest(const std::shared_ptr<const ledger::core::HttpRequest>& self);
                virtual api::HttpMethod getMethod() override;

                virtual std::unordered_map<std::string, std::string> getHeaders() override;

                virtual std::vector<uint8_t> getBody() override;

                virtual std::string getUrl() override;

                virtual ~ApiRequest();

                virtual void complete(const std::shared_ptr<api::HttpUrlConnection> &response,
                              const optional<api::Error> &error) override;

                Future<std::shared_ptr<api::HttpUrlConnection>> getFuture() const;

            private:
                std::shared_ptr<const ledger::core::HttpRequest> _self;
                Promise<std::shared_ptr<api::HttpUrlConnection>> _promise;
            };
        };

        class HttpClient {
        public:
            HttpClient(const std::string& baseUrl,
                       const std::shared_ptr<api::HttpClient> &client,
                       const std::shared_ptr<api::ExecutionContext> &context
            );
            HttpRequest GET(const std::string& path, const std::unordered_map<std::string, std::string>& headers = {});
            HttpRequest PUT(const std::string& path, const std::vector<uint8_t> &body, const std::unordered_map<std::string, std::string>& headers = {});
            HttpRequest DEL(const std::string& path, const std::unordered_map<std::string, std::string>& headers = {});
            HttpRequest POST(const std::string& path, const std::vector<uint8_t> &body, const std::unordered_map<std::string, std::string>& headers = {});
            void addHeader(const std::string& key, const std::string& value);
            void removeHeader(const std::string& key);
        private:
            HttpRequest createRequest(api::HttpMethod method,
                                      const std::string& path,
                                      const std::experimental::optional<std::vector<uint8_t >> body,
                                      const std::unordered_map<std::string, std::string>& headers
            );

        private:
            std::string _baseUrl;
            std::shared_ptr<api::HttpClient> _client;
            std::shared_ptr<api::ExecutionContext> _context;
            std::unordered_map<std::string, std::string> _headers;
        };
    }
}


#endif //LEDGER_CORE_HTTPCLIENT_HPP
