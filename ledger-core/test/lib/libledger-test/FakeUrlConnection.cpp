#include <core/api/HttpReadBodyResult.hpp>
#include <core/api/Error.hpp>
#include <core/utils/Optional.hpp>

#include "FakeUrlConnection.hpp"

namespace ledger {
    namespace core {
        namespace test {
            std::shared_ptr<FakeUrlConnection> FakeUrlConnection::fromString(const std::string& responseData) {
                UrlConnectionData data;
                data.body = responseData;
                data.statusCode = 200;
                data.statusText = "OK";
                return std::make_shared<FakeUrlConnection>(data);
            }

            FakeUrlConnection::FakeUrlConnection(const UrlConnectionData& data)
                : _data(data) {};

            int32_t FakeUrlConnection::getStatusCode() {
                return _data.statusCode;
            };

            std::string FakeUrlConnection::getStatusText() {
                return _data.statusText;
            };

            std::unordered_map<std::string, std::string> FakeUrlConnection::getHeaders() {
                return _data.headers;
            };

            api::HttpReadBodyResult FakeUrlConnection::readBody() {
                std::vector<uint8_t> bindata((const uint8_t*)_data.body.data(), (const uint8_t*)_data.body.data() + _data.body.size());
                api::HttpReadBodyResult result(std::experimental::nullopt, bindata);
                return result;
            };
        }
    }
}
