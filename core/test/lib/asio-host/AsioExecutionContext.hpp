#pragma once
#include <api/ExecutionContext.hpp>

namespace ledger {
    namespace core {
        namespace api {
            class Runnable;
        };
    }
}
//Pimpl to not expose dependency on asio.hpp
class AsioExecutionContextImpl;

class AsioExecutionContext : public ledger::core::api::ExecutionContext {
public:
    AsioExecutionContext();
    void execute(const std::shared_ptr<ledger::core::api::Runnable> & runnable) override;

    void delay(const std::shared_ptr<ledger::core::api::Runnable> & runnable, int64_t millis) override;

    void run();

    std::shared_ptr<AsioExecutionContextImpl> getPimpl();
private:
    std::shared_ptr<AsioExecutionContextImpl> pimpl;
};