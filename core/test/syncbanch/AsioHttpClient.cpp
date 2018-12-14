#include "AsioHttpClient.hpp"
#include "AsioExecutionContext.hpp"
#include "RequestResponce.hpp"
#include <api/ExecutionContext.hpp>
#include <api/HttpUrlConnection.hpp>

using namespace ledger::core;

AsioHttpClient::AsioHttpClient(const std::shared_ptr<AsioExecutionContext> &context) {
    _context = context;
}

void AsioHttpClient::execute(const std::shared_ptr<api::HttpRequest> &request) {
    auto r = std::make_shared<RequestResponse>(_context->_io_service, request);
    r->execute();
}
