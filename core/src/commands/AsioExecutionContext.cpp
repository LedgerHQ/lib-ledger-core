#include <memory>
#include "AsioExecutionContext.hpp"
#include "api/Runnable.hpp"
#include <iostream>

namespace ledger {
    namespace core {

        AsioExecutionContext::AsioExecutionContext()
            : _work(asio::make_work_guard(_io_service)){
        }

        void AsioExecutionContext::execute(const std::shared_ptr<ledger::core::api::Runnable>& runnable) {
            _io_service.post([runnable]() { runnable->run(); });
        };

        void AsioExecutionContext::delay(const std::shared_ptr<ledger::core::api::Runnable>& runnable, int64_t millis) {

        }

        void AsioExecutionContext::start() {
            _executionThread = std::make_unique<std::thread>([this]() {
                try {
                    _io_service.run();
                }
                catch (std::exception const& r) {
                    std::cout << "Exception in ExecutionContext loop" << r.what() << std::endl;
                }
                catch (...) {
                    std::cout << "Something bad happened" << std::endl;
                }
            });
        }

        void AsioExecutionContext::stop() {
            _work.reset();
            _executionThread->join();
        }
    }
}