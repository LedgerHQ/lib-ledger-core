#pragma once
#include <unordered_map>
#include <memory>
#include "api/HttpClient.hpp"
#include "FakeUrlConnection.hpp"
#include "ExplorerStorage.hpp"

namespace ledger {
    namespace core {
        namespace test {

            class HttpClientOnFakeExplorer : public api::HttpClient {
            public:
                HttpClientOnFakeExplorer(std::shared_ptr<ExplorerStorage> explorer);
                void execute(const std::shared_ptr<api::HttpRequest>& request) override;
            private:
                std::shared_ptr<ExplorerStorage> _explorer;
            };

        }
    }
}