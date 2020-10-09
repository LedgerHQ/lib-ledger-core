#include <memory>
#include <chrono>
#include <unordered_map>
#include <fmt/format.h>
#include "api/Error.hpp"
#include "api/HttpRequest.hpp"

#include <api/HttpUrlConnection.hpp>
#include <api/HttpReadBodyResult.hpp>
#include <api/HttpMethod.hpp>

#include <ledger/core/utils/Exception.hpp>
#include "NativeThreadDispatcher.hpp"
#include "CppHttpLibClient.hpp"

namespace ledger {
    namespace core {
        namespace test {

            class UrlConnection : public core::api::HttpUrlConnection {
            public:
                UrlConnection(int32_t statusCode,
                            std::string statusText,
                            std::unordered_map<std::string, std::string> headers,
                            std::string body
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
                    //std::vector<uint8_t> body((uint8_t *)_body.c_str(), (uint8_t *)(_body.c_str() + _body.size()));
                    std::vector<uint8_t> body(_body.begin(), _body.end());
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
                std::string _body;
            };

            void generateCacheFile(const std::string& url, const std::string& response)
            {
                auto filename = fmt::format("http_cache_{}.hpp" ,std::chrono::system_clock::now().time_since_epoch().count());
                std::ofstream output(filename);
                if (output.is_open())
                {
                    output << "namespace HTTP_CACHE_XXXX {"<< std::endl;
                    output << "const std::string URL = R\"(" << url << ")\";" << std::endl;
                    output << "const std::string BODY= R\"(" << response << ")\";" << std::endl;
                    output << "}" << std::endl;
                }
            }

            void CppHttpLibClient::execute(const std::shared_ptr<core::api::HttpRequest> &request) {
            _context->execute(make_runnable([=] () {
                auto url = request->getUrl();
                std::size_t sep;
                std::size_t sepIgnored = url.find("//");
                if (sepIgnored == std::string::npos) {
                    sep = url.find('/');
                }
                else {
                    sep = url.find('/', sepIgnored+2);
                }
                auto baseUrl = url.substr(0, sep);
                auto path = url.substr(sep);
                
                httplib::Client cli(baseUrl.c_str());
                httplib::Headers headers;
                for (const auto& header : request->getHeaders()) {
                    headers.insert(std::make_pair(header.first, header.second));
                }
                cli.set_default_headers(headers);
                std::string body;
                if (request->getBody().size() > 0) {
                    std::stringstream bodyStream;
                    for (auto byte : request->getBody()) {
                        bodyStream << (char )byte;
                    }
                    body = bodyStream.str();
                }

                if (_logger) { 
                    _logger->debug("CppHttpLibClient: quering {} {}", api::to_string(request->getMethod()), request->getUrl());
                }
                
                httplib::Result res(nullptr, httplib::Error::Unknown);
                switch(request->getMethod())
                {
                    case core::api::HttpMethod::POST:
                        res = cli.Post(path.c_str(), body, "text/plain");
                    break;
                    case core::api::HttpMethod::GET:
                        res = cli.Get(path.c_str());
                    break;
                    case core::api::HttpMethod::PUT:
                        res = cli.Put(path.c_str(), body, "text/plain");
                    break;
                    case core::api::HttpMethod::DEL:
                        res = cli.Delete(path.c_str());
                    break;
                    default:
                        throw make_exception(api::ErrorCode::RUNTIME_ERROR, "unknown http method");
                }
                
                if (_logger) {
                    _logger->debug("CppHttpLibClient: done {} {} - {}", api::to_string(request->getMethod()), request->getUrl(), res->status);
                }

                if (res.error() != httplib::Error::Success) {
                    request->complete(nullptr, std::experimental::optional<core::api::Error>(
                        core::api::Error(core::api::ErrorCode::HTTP_ERROR, 
                        "HTTP ERROR")));       
                }
                else if (res->status < 200 || res->status >= 300 ) {
                    std::unordered_map<std::string, std::string> hdrs;
                    for (const auto& header : res->headers) {
                        hdrs.insert(std::make_pair(header.first, header.second));
                    }
                    auto connection = std::make_shared<UrlConnection>(
                        res->status,
                        res->reason,
                        hdrs,
                        res->body
                        );
                    request->complete(connection, std::experimental::optional<core::api::Error>(
                        core::api::Error(core::api::ErrorCode::HTTP_ERROR, 
                        fmt::format("{}: {}", res->status, res->reason)))); 
                }
                else {
                    std::unordered_map<std::string, std::string> hdrs;
                    for (const auto& header : res->headers) {
                        hdrs.insert(std::make_pair(header.first, header.second));
                    }
                    auto connection = std::make_shared<UrlConnection>(
                        res->status,
                        res->reason,
                        hdrs,
                        res->body
                        );
                    if (_generateCacheFile) {
                        generateCacheFile(url, res->body);
                    }
                    request->complete(connection, std::experimental::optional<core::api::Error>());
                }
            })); 
        }
    
        }
    }
}