#pragma once
#include "AsioExecutionContextImpl.hpp"
#include <api/Runnable.hpp>

void AsioExecutionContextImpl::execute(const std::shared_ptr<ledger::core::api::Runnable> & runnable) {
    q.push(runnable);
};

void AsioExecutionContextImpl::delay(const std::shared_ptr<ledger::core::api::Runnable> & runnable, int64_t millis) {
    throw 2;
}

bool AsioExecutionContextImpl::runOne() {
    if (q.empty())
        return false;
    auto x = q.front();
    q.pop();
    x->run();
    return true;
};

void AsioExecutionContextImpl::run() {
    while (true) {
        while (runOne());
        if (_io_service.run() == 0)
            return;
        _io_service.restart();
    }
}