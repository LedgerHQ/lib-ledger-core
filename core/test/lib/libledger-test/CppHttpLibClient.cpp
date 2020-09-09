#include <memory>
#include <unordered_map>
#include "api/Error.hpp"
#include "api/HttpRequest.hpp"
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
                    std::vector<uint8_t> body((uint8_t *)_body.c_str(), (uint8_t *)(_body.c_str() + _body.size()));
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
                
                httplib::Result res = 
                    (request->getMethod() == core::api::HttpMethod::POST) ? cli.Post(path.c_str(), body, "text/plain") :
                    (request->getMethod() == core::api::HttpMethod::GET) ?  cli.Get(path.c_str()) :
                    (request->getMethod() == core::api::HttpMethod::PUT) ?  cli.Put(path.c_str(), body, "text/plain") :
                                                                            cli.Delete(path.c_str());
                
                if (res.error() != httplib::Error::Success || res->status < 200 || res->status >= 300 ) {
                    request->complete(nullptr, std::experimental::optional<core::api::Error>(
                        core::api::Error(core::api::ErrorCode::HTTP_ERROR, res->reason)));       
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
                    request->complete(connection, std::experimental::optional<core::api::Error>());
                }
            })); 
        }
    
        }
    }
}