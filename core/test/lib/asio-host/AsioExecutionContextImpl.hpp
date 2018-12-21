#pragma once
#include <asio.hpp>
#include <memory>
#include <queue>

namespace ledger {
    namespace core {
        namespace api {
            class Runnable;
        };
    }
}

class AsioExecutionContextImpl {
public:
    void execute(const std::shared_ptr<ledger::core::api::Runnable> & runnable);

    void delay(const std::shared_ptr<ledger::core::api::Runnable> & runnable, int64_t millis);

    void run();
private:
    bool runOne();
public:
    asio::io_service _io_service;
private:
    std::queue<std::shared_ptr<ledger::core::api::Runnable>> q;
};