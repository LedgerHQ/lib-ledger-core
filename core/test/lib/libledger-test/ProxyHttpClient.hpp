#pragma once
#include "api/HttpClient.hpp"
#include <unordered_map>
#include "FakeUrlConnection.hpp"

namespace ledger {
    namespace core {
        namespace test {

            class ProxyHttpClient : public api::HttpClient {
            public:
                ProxyHttpClient(std::shared_ptr<api::HttpClient> httpClient);
                void execute(const std::shared_ptr<api::HttpRequest>& request) override;
                void addCache(const std::string& url, const std::string& body);
            private:
                std::unordered_map<std::string, std::shared_ptr<FakeUrlConnection>> _cache;
                std::shared_ptr<api::HttpClient> _httpClient;
            };

        }
    }
}
