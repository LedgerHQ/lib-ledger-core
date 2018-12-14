#pragma once
#include "AsioExecutionContext.hpp"
#include <api/Runnable.hpp>

void AsioExecutionContext::execute(const std::shared_ptr<ledger::core::api::Runnable> & runnable) {
    q.push(runnable);
};

void AsioExecutionContext::delay(const std::shared_ptr<ledger::core::api::Runnable> & runnable, int64_t millis) {
    throw 2;
}

bool AsioExecutionContext::runOne() {
    if (q.empty())
        return false;
    auto x = q.front();
    q.pop();
    x->run();
    return true;
};

void AsioExecutionContext::run() {
    while (true) {
        while (runOne());
        if (_io_service.run() == 0)
            return;
        _io_service.restart();
    }
}