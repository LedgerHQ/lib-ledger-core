#include <memory>
#include "AsioExecutionContext.hpp"
#include "api/Runnable.hpp"
#include <iostream>

namespace ledger {
    namespace core {

        void AsioExecutionContext::execute(const std::shared_ptr<ledger::core::api::Runnable>& runnable) {
            _io_service.post([runnable]() { runnable->run(); });
        };

        void AsioExecutionContext::delay(const std::shared_ptr<ledger::core::api::Runnable>& runnable, int64_t millis) {

        }

        void AsioExecutionContext::start() {
            _shouldStop = false;
            _executionThread = std::make_unique<std::thread>([this]() {
                while (true) {
                    try {
                        if ((_io_service.run() == 0) && _shouldStop) // exit only when there are nothing more to do
                            return;
                        _io_service.reset();
                    }
                    catch (std::exception const& r) {
                        std::cout << "Exception in ExecutionContext loop" << r.what() << std::endl;
                    }
                    catch (...) {
                        std::cout << "Something bad happened" << std::endl;
                    }
                }
                });
        }

        void AsioExecutionContext::stop() {
            _shouldStop = true;
            _executionThread->join();
        }
    }
}