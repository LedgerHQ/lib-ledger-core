#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <functional>
#include <api/HttpClient.hpp>
#include <api/HttpUrlConnection.hpp>
#include <api/HttpRequest.hpp>
#include <api/HttpReadBodyResult.hpp>
#include <api/HttpMethod.hpp>
#include <api/ExecutionContext.hpp>
#include <httplib.h>


namespace ledger {
    namespace core {
        namespace test {

            class CppHttpLibClient : public core::api::HttpClient {
            public:
                CppHttpLibClient(const std::shared_ptr<core::api::ExecutionContext>& context) : _context(context) {};
                void execute(const std::shared_ptr<core::api::HttpRequest> &request) override;
            private:
                std::shared_ptr<core::api::ExecutionContext> _context;
            };

        }
    }
}