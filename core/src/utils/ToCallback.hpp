#pragma once
#include <stlab/concurrency/future.hpp>
#include <string>
#include <memory>
#include <utils/Exception.hpp>

#include <api/ErrorCodeCallback.hpp>
#include <api/ErrorCode.hpp>

namespace ledger {
    namespace core {

        template<typename T, typename C>
        void toCallback(const stlab::future<T>& futur, const std::shared_ptr<C>& callback)
        {
            futur.recover([callback](stlab::future<T> x) {
                try {
                    callback->onCallback(std::ledger_exp::optional<T>(*x.get_try()), std::ledger_exp::optional<api::Error>());
                }
                catch (const Exception & e) {
                    callback->onCallback(std::ledger_exp::optional<T>(), std::ledger_exp::optional<api::Error>(e.toApiError()));
                }
                catch (const std::exception & e) {
                    callback->onCallback(std::ledger_exp::optional<T>(), std::ledger_exp::optional<api::Error>(api::Error(api::ErrorCode::RUNTIME_ERROR, e.what())));
                }
                catch (...) {
                    callback->onCallback(std::ledger_exp::optional<T>(), std::ledger_exp::optional<api::Error>(api::Error(api::ErrorCode::UNKNOWN, "Unknow error")));
                }
                });
        }

        template<typename T, typename C>
        void toCallbackPtr(const stlab::future<std::shared_ptr<T>>& futur, const std::shared_ptr<C>& callback)
        {
            futur.recover([callback](stlab::future<std::shared_ptr<T>> x) {
                try {
                    callback->onCallback(*x.get_try(), std::ledger_exp::optional<api::Error>());
                }
                catch (const Exception & e) {
                    callback->onCallback(nullptr, std::ledger_exp::optional<api::Error>(e.toApiError()));
                }
                catch (const std::exception & e) {
                    callback->onCallback(nullptr, std::ledger_exp::optional<api::Error>(api::Error(api::ErrorCode::RUNTIME_ERROR, e.what())));
                }
                catch (...) {
                    callback->onCallback(nullptr, std::ledger_exp::optional<api::Error>(api::Error(api::ErrorCode::UNKNOWN, "Unknow error")));
                }
                });
        }

        void toErrorCodeCallback(const stlab::future<void>& futur, const std::shared_ptr<api::ErrorCodeCallback>& callback);
    }
}