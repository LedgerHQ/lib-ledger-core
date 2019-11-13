#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include "commands.pb.h"
#include "ubinder/function_types.h"
#include "ubinder/cpp_wrapper.hpp"
#include "async/Future.hpp"

namespace ledger {
    namespace core {
        class WalletPool;

        class AsioExecutionContext;
        class BitcoinLikeCommandProcessor;
        class UtilsCommandProcessor;
        class LogPrinter;
        class PathResolver;
        class ThreadDispatcher;
        class WrapperHttpClient;
        
        class LibCoreCommands {
        public:
            LibCoreCommands();
            void OnRequest(std::vector<uint8_t>&& data, std::function<void(std::vector<uint8_t>&&)>&& callback);
            void OnNotification(std::vector<uint8_t>&& data);
            void OnExit();
        private:
            std::once_flag _startExecutionContext;

            // services
            std::shared_ptr<AsioExecutionContext> _executionContext;
            std::shared_ptr<WrapperHttpClient> _httpClient;
            std::shared_ptr<PathResolver> _pathResolver;
            std::shared_ptr<ThreadDispatcher> _threadDispathcher;
            std::shared_ptr<LogPrinter> _logPrinter;
            
            std::shared_ptr<WalletPool> _walletPool;

            //processors
            std::unique_ptr<BitcoinLikeCommandProcessor> _bitcoinLikeProcessor;
            std::unique_ptr<UtilsCommandProcessor> _utilsProcessor;
        private:
            Future<message::CoreResponse> processRequest(const message::CoreRequest&& request);
        };

        static ubinder::CppWrapper<ledger::core::LibCoreCommands> CppWrapperInstance;

        void OnRequestFunc(uint32_t request, const char* data, size_t dataSize) {
            CppWrapperInstance.onRequest(request, data, dataSize);
        }

        void OnResponseFunc(uint32_t request, const char* data, size_t dataSize) {
            CppWrapperInstance.onResponse(request, data, dataSize);
        }

        void OnNotificationFunc(const char* data, size_t dataSize) {
            CppWrapperInstance.onNotification(data, dataSize);
        }

        void OnExit() {
            CppWrapperInstance.onExit();
        }
    }
}
