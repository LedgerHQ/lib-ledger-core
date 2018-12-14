#pragma once

#include <api/HttpClient.hpp>
#include <api/ExecutionContext.hpp>
#include <api/HttpMethod.hpp>
#include <api/HttpReadBodyResult.hpp>
#include <api/HttpUrlConnection.hpp>
#include <api/HttpRequest.hpp>

class AsioExecutionContext;

class AsioHttpClient : public ledger::core::api::HttpClient {
public:
    AsioHttpClient(const std::shared_ptr<AsioExecutionContext>& context);
    void execute(const std::shared_ptr<ledger::core::api::HttpRequest> &request) override;
private:
    std::shared_ptr<AsioExecutionContext> _context;
};
