#include "AsioExecutionContext.hpp"
#include "commands/CommandProcessor.h"
#include "ledger-core.h"
#include "collections/DynamicObject.hpp"
#include "async/Future.hpp"
#include "utils/Try.hpp"
#include "commands/BitcoinLikeProcessor.h"
#include "commands.pb.h"
#include "core_configuration.pb.h"
#include "commands/PathResolver.hpp"
#include "commands/ThreadDispatcher.hpp"
#include "commands/LogPrinter.hpp"
#include "wallet/pool/WalletPool.hpp"
#include "WrapperHttpClient.hpp"
#include "wallet/pool/WalletPool.hpp"
#include "ubinder/wrapper_interface.h"


namespace ledger {
    namespace core {
        using namespace message;

        LibCoreCommands::LibCoreCommands() 
            : _executionContext(std::make_shared<AsioExecutionContext>()) {
            _httpClient = std::make_shared<WrapperHttpClient>([this](std::vector<uint8_t>&& req, std::function<void(std::vector<uint8_t>&&)>&& callback) {
                CppWrapperInstance.SendRequest(std::move(req), std::move(callback));
                });
            _pathResolver = std::make_shared<PathResolver>();
            _threadDispathcher = std::make_shared<ThreadDispatcher>(_executionContext);
            _logPrinter = std::make_shared<LogPrinter>(_executionContext);
        }

        void LibCoreCommands::OnRequest(std::vector<uint8_t>&& data, std::function<void(std::vector<uint8_t>&&)>&& callback) {
            std::call_once(_startExecutionContext, [this]() { _executionContext->start(); });
            CoreRequest request;
            if (data.size() > 0) {
                request.ParseFromArray(&data[0], data.size());
            }
            return processRequest(std::move(request))
                .onComplete(_executionContext, [cb{ std::move(callback) }](const Try<CoreResponse>& t) -> void {
                    if (t.isSuccess()) {
                        auto& val = t.getValue();
                        std::vector<uint8_t> data(val.ByteSize());
                        if (data.size() > 0) {
                            val.SerializeToArray(&data[0], data.size());
                        }
                        cb(std::move(data));
                    }
                    else {
                        CoreResponse response;
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

        Future<CoreResponse> LibCoreCommands::processRequest(CoreRequest&& request) {
            switch (request.request_type())
            {
            case CoreRequestType::GET_VERSION: {
                CoreResponse resp;
                GetVersionResponse getVersion;
                getVersion.set_major(VERSION_MAJOR);
                getVersion.set_minor(VERSION_MINOR);
                getVersion.set_patch(VERSION_PATCH);
                resp.set_response_body(getVersion.SerializeAsString());
                return Future<CoreResponse>::successful(resp);
            }
            case CoreRequestType::SET_CONFIGURATION: {
                LibCoreConfiguration newConfiguration;
                if (!newConfiguration.ParseFromString(request.request_body())) {
                    CoreResponse resp;
                    resp.set_error("Can't parse configuration message");
                    return Future<CoreResponse>::successful(resp);
                }
                _pathResolver->setWorkingDir(newConfiguration.working_dir());
                auto config = api::DynamicObject::newInstance();
                auto dbBackend = api::DatabaseBackend::getSqlite3Backend();
                std::string password;
                if (!_walletPool) {
                    _walletPool = WalletPool::newInstance(
                        "cmd",
                        password,
                        _httpClient,
                        nullptr,
                        _pathResolver,
                        _logPrinter,
                        _threadDispathcher,
                        nullptr,
                        dbBackend,
                        config);
                    _bitcoinLikeProcessor = std::make_unique<BitcoinLikeCommandProcessor>(_walletPool);
                    CoreResponse resp;
                    return Future<CoreResponse>::successful(resp);
                }
                else {
                    //_walletPool->updateWalletConfig()
                }
            }
            case CoreRequestType::BITCOIN_REQUEST: {
                return _bitcoinLikeProcessor->processRequest(request.request_body())
                    .map<CoreResponse>(_walletPool->getContext(), [](const std::string& buff) {
                        CoreResponse resp;
                        resp.set_response_body(buff);
                        std::string temp_debub = resp.SerializeAsString();
                        return resp;
                    });
            }
            default:
                CoreResponse resp;
                resp.set_error("unknown message type");
                return Future<CoreResponse>::successful(resp);
            }
        }
    }
}

#ifdef __cplusplus
extern "C" {
#endif

WRAPPER_EXPORT     
    void initWrapper(
        ::RequestResponse sendRequest,
        ::RequestResponse sendResponse,
        ::Notification sendNotification,
        ::RequestResponse* onRequest,
        ::RequestResponse* onResponse,
        ::Notification* onNotification) {
        ledger::core::CppWrapperInstance.sendRequest = sendRequest;
        ledger::core::CppWrapperInstance.sendResponse = sendResponse;
        ledger::core::CppWrapperInstance.sendNotification = sendNotification;
        *onRequest = &ledger::core::OnRequestFunc;
        *onResponse = &ledger::core::OnResponseFunc;
        *onNotification = &ledger::core::OnNotificationFunc;
    }
    
#ifdef __cplusplus
}
#endif