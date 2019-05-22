#pragma once
#include <memory>
#include <functional>
#include <vector>
#include "api/HttpClient.hpp"

namespace ledger {
    namespace core {
        namespace api {
            class HttpRequest;
        }
        
        class WrapperHttpClient : public api::HttpClient {
        public:
            typedef std::function<void(std::vector<uint8_t>&&, std::function<void(std::vector<uint8_t>&&)>)> OnRequest;
        public:
            WrapperHttpClient(const OnRequest& onRequest);
            void execute(const std::shared_ptr<api::HttpRequest>& request) override;
        private:
            const OnRequest _onRequest;
        };
} }

