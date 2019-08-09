#include "FakeHttpClient.hpp"
#include "api/HttpRequest.hpp"
#include <memory>
#include "api/Error.hpp"

namespace ledger {
    namespace core {
        namespace test {
            void FakeHttpClient::execute(const std::shared_ptr<api::HttpRequest>& request) {
                auto it = _behavior.find(request->getUrl());
                if (it != _behavior.end()) {
                    request->complete(it->second, std::experimental::nullopt);
                    return;
                }
                request->complete(std::shared_ptr<api::HttpUrlConnection>(), api::Error(api::ErrorCode::BLOCK_NOT_FOUND, "Block not found"));
            }

            void FakeHttpClient::setBehavior(const std::unordered_map<std::string, std::shared_ptr<FakeUrlConnection>>& behavior) {
                _behavior = behavior;
            }
        }
    }
}