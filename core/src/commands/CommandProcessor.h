#pragma once
#include <vector>
#include <functional>
#include <memory>
#include "messages.pb.h"
#include "ubinder/wrapper_interface.h"
#include "ubinder/function_types.h"
#include "ubinder/cpp_wrapper.hpp"
#include "async/Future.hpp"

namespace ledger {
    namespace core {
        class WalletPool;

        class AsioExecutionContext;
        class BitcoinLikeCommandProcessor;
        
        class LibCoreCommands {
        public:
            LibCoreCommands();
            void OnRequest(std::vector<uint8_t>&& data, std::function<void(std::vector<uint8_t>&&)>&& callback);
            void OnNotification(std::vector<uint8_t>&& data);
        private:
            std::shared_ptr<AsioExecutionContext> _executionContext;
            std::once_flag _startExecutionContext;
            std::unique_ptr<BitcoinLikeCommandProcessor> _bitcoinLikeProcessor;
            std::shared_ptr<WalletPool> _walletPool;
        private:
            Future<lib_core_proto::LibCoreResponse> processRequest(lib_core_proto::LibCoreRequest&& request);
        };

        static ubinder::CppWrapper<ledger::core::LibCoreCommands> CppWrapperInstance;

        void OnRequestFunc(const void* request, const char* data, size_t dataSize) {
            CppWrapperInstance.onRequest(request, data, dataSize);
        }

        void OnResponseFunc(const void* request, const char* data, size_t dataSize) {
            CppWrapperInstance.onResponse(request, data, dataSize);
        }

        void OnNotificationFunc(const char* data, size_t dataSize) {
            CppWrapperInstance.onNotification(data, dataSize);
        }
    }
}


extern "C" {
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
}
