#pragma once
#include <api/ExecutionContext.hpp>
#include <asio.hpp>
#include <queue>

namespace ledger {
    namespace core {
        namespace api {
            class Runnable;
        };
    }
}

class AsioExecutionContext : public ledger::core::api::ExecutionContext {
public:
    void execute(const std::shared_ptr<ledger::core::api::Runnable> & runnable) override;

    void delay(const std::shared_ptr<ledger::core::api::Runnable> & runnable, int64_t millis) override;

    void run();
private:
    bool runOne();
public:
    asio::io_service _io_service;
private:
    std::queue<std::shared_ptr<ledger::core::api::Runnable>> q;
};