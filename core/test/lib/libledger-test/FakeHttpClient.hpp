#pragma once
#include "api/HttpClient.hpp"
#include <unordered_map>
#include "FakeUrlConnection.hpp"

namespace ledger {
namespace core {
namespace test {

    class FakeHttpClient : public api::HttpClient {
    public:
        void execute(const std::shared_ptr<api::HttpRequest>& request) override;
        void setBehavior(const std::unordered_map<std::string, std::shared_ptr<FakeUrlConnection>>& behavior);
    private:
        std::unordered_map<std::string, std::shared_ptr<FakeUrlConnection>> _behavior;
    };

}
}
}