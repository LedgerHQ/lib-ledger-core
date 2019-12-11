#pragma once

#include <memory>

#include <core/api/HttpUrlConnection.hpp>

namespace ledger {
    namespace core {
        namespace test {
            struct UrlConnectionData {
                int32_t statusCode;
                std::string statusText;
                std::unordered_map<std::string, std::string> headers;
                std::string body;
            };

            class FakeUrlConnection : public api::HttpUrlConnection {
            public:
                static std::shared_ptr<FakeUrlConnection> fromString(const std::string& responseData);
                FakeUrlConnection(const UrlConnectionData& data);
                int32_t getStatusCode() override;
                std::string getStatusText() override;
                std::unordered_map<std::string, std::string> getHeaders() override;
                api::HttpReadBodyResult readBody() override;
            private:
                UrlConnectionData _data;
            };
        }
    }
}
