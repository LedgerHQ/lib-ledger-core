#include "AsioExecutionContext.hpp"
#include "CommandProcessor.h"
#include "ledger-core.h"
#include "collections/DynamicObject.hpp"
#include "async/Future.hpp"
#include "utils/Try.hpp"
#include "BitcoinLikeProcessor.h"
#include "messages.pb.h"
#include "PathResolver.hpp"
#include "ThreadDispatcher.hpp"
#include "LogPrinter.hpp"
#include "wallet/pool/WalletPool.hpp"
#include "WrapperHttpClient.hpp"
#include "wallet/pool/WalletPool.hpp"


namespace ledger {
    namespace core {
        using namespace lib_core_proto;

        LibCoreCommands::LibCoreCommands() 
            : _executionContext(std::make_shared<AsioExecutionContext>()) {
        }

        void LibCoreCommands::OnRequest(std::vector<uint8_t>&& data, std::function<void(std::vector<uint8_t>&&)>&& callback) {
            std::call_once(_startExecutionContext, [this]() {
                _executionContext->start();
                auto httpClient = std::make_shared<WrapperHttpClient>([this](std::vector<uint8_t>&& req, std::function<void(std::vector<uint8_t>&&)>&& callback) {
                    CppWrapperInstance.SendRequest(std::move(req), std::move(callback));
                    });
                auto pathResolver = std::make_shared<PathResolver>();
                auto threadDispathcher = std::make_shared<ThreadDispatcher>(_executionContext);
                auto logPrinter = std::make_shared<LogPrinter>(_executionContext);
                auto config = api::DynamicObject::newInstance();
                auto dbBackend = api::DatabaseBackend::getSqlite3Backend();
                std::string password;
                // we use wallet pool as a container for all services
                _walletPool = WalletPool::newInstance(
                    "cmd-wallet",
                    password,
                    httpClient,
                    nullptr,
                    pathResolver,
                    logPrinter,
                    threadDispathcher,
                    nullptr,
                    dbBackend,
                    config);
                _bitcoinLikeProcessor = std::make_unique<BitcoinLikeCommandProcessor>(_walletPool);
            });
            LibCoreRequest request;
            if (data.size() > 0) {
                request.ParseFromArray(&data[0], data.size());
            }
            return processRequest(std::move(request))
                .onComplete(_executionContext, [cb{ std::move(callback) }](const Try<LibCoreResponse>& t) -> void {
                    if (t.isSuccess()) {
                        auto& val = t.getValue();
                        std::vector<uint8_t> data(val.ByteSize());
                        if (data.size() > 0) {
                            val.SerializeToArray(&data[0], data.size());
                        }
                        cb(std::move(data));
                    }
                    else {
                        LibCoreResponse response;
                        response.set_error(t.getFailure().getMessage());
                        std::vector<uint8_t> data(response.ByteSize());
                        if (data.size() > 0) {
                            response.SerializeToArray(&data[0], data.size());
                        }
                        cb(std::move(data));
                    }
                });
        }

        void LibCoreCommands::OnNotification(std::vector<uint8_t>&& data) {

        }

        Future<LibCoreResponse> LibCoreCommands::processRequest(LibCoreRequest&& request) {
            
            switch (request.request_type())
            {
            case LibCoreRequestType::GET_VERSION: {
                LibCoreResponse resp;
                GetVersionResponse getVersion;
                getVersion.set_major(VERSION_MAJOR);
                getVersion.set_minor(VERSION_MINOR);
                getVersion.set_patch(VERSION_PATCH);
                resp.set_response_body(getVersion.SerializeAsString());
                return Future<LibCoreResponse>::successful(resp);
            }
            case LibCoreRequestType::BITCOIN_REQUEST: {
                return _bitcoinLikeProcessor->processRequest(request.request_body())
                    .map<LibCoreResponse>(_walletPool->getContext(), [](const std::string& buff) {
                        LibCoreResponse resp;
                        resp.set_response_body(buff);
                        std::string temp_debub = resp.SerializeAsString();
                        return resp;
                    });
            }
            default:
                LibCoreResponse resp;
                resp.set_error("unknown message type");
                return Future<LibCoreResponse>::successful(resp);
            }
        }
    }
}