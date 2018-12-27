#pragma once
#include "AsioExecutionContext.hpp"
#include "AsioExecutionContextImpl.hpp"
#include <api/Runnable.hpp>

AsioExecutionContext::AsioExecutionContext() {
    pimpl = std::make_shared<AsioExecutionContextImpl>();
}

void AsioExecutionContext::execute(const std::shared_ptr<ledger::core::api::Runnable> & runnable) {
    pimpl->execute(runnable);
};

void AsioExecutionContext::delay(const std::shared_ptr<ledger::core::api::Runnable> & runnable, int64_t millis) {
    pimpl->delay(runnable, millis);
}

void AsioExecutionContext::run() {
    pimpl->run();
}

std::shared_ptr<AsioExecutionContextImpl> AsioExecutionContext::getPimpl() {
    return pimpl;
}