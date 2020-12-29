#include "ProxyHttpClient.hpp"
#include "api/HttpRequest.hpp"
#include <memory>
#include "api/Error.hpp"

namespace ledger {
    namespace core {
        namespace test {
            ProxyHttpClient::ProxyHttpClient(std::shared_ptr<api::HttpClient> httpClient) 
                : _httpClient(httpClient)
            {}

            void ProxyHttpClient::execute(const std::shared_ptr<api::HttpRequest>& request) {
                auto it = _cache.find(request->getUrl());
                if (it != _cache.end()) {
                    std::cout << "get response from cache : " << request->getUrl() << std::endl;
                    request->complete(it->second, std::experimental::nullopt);
                    return;
                }
                _httpClient->execute(request);
            }

            void ProxyHttpClient::addCache(const std::string& url, const std::string& body) {
                _cache.emplace(url, FakeUrlConnection::fromString(body));
            }
        }
    }
}