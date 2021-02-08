#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <functional>
#include <api/HttpClient.hpp>

#include <api/HttpRequest.hpp> 
#include <api/ExecutionContext.hpp>
#include <spdlog/logger.h>
#include <httplib.h>


namespace ledger {
    namespace core {
        namespace test {

            class CppHttpLibClient : public core::api::HttpClient {
            public:
                CppHttpLibClient(const std::shared_ptr<core::api::ExecutionContext>& context) : 
                _context(context), _logger(nullptr), _generateCacheFile(false)
                {};
                
                void execute(const std::shared_ptr<core::api::HttpRequest> &request) override;

                void setLogger(const std::shared_ptr<spdlog::logger>& logger) {
                    _logger = logger;
                }

                void setGenerateCacheFile(bool generateCacheFile) {
                    _generateCacheFile = generateCacheFile;
                }

            private:
                std::shared_ptr<core::api::ExecutionContext> _context;
                std::shared_ptr<spdlog::logger> _logger;
                bool _generateCacheFile;
            };

        }
    }
}