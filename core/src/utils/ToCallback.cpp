#include "utils/ToCallback.hpp"
#include "utils/Exception.hpp"
#include "utils/optional.hpp"
#include <api/Error.hpp>
#include <api/ErrorCode.hpp>
#include <memory>



namespace ledger {
    namespace core {
        void toErrorCodeCallback(const stlab::future<void>& futur, const std::shared_ptr<api::ErrorCodeCallback>& callback)
        {
            futur.recover([callback](stlab::future<void> x) {
                try {
                    x.get_try();
                    callback->onCallback(api::ErrorCode::FUTURE_WAS_SUCCESSFULL, std::ledger_exp::optional<api::Error>());
                }
                catch (const Exception & e) {
                    callback->onCallback(std::ledger_exp::optional<api::ErrorCode>(), std::ledger_exp::optional<api::Error>(e.toApiError()));
                }
                catch (const std::exception & e) {
                    callback->onCallback(std::ledger_exp::optional<api::ErrorCode>(), std::ledger_exp::optional<api::Error>(api::Error(api::ErrorCode::RUNTIME_ERROR, e.what())));
                }
                catch (...) {
                    callback->onCallback(std::ledger_exp::optional<api::ErrorCode>(), std::ledger_exp::optional<api::Error>(api::Error(api::ErrorCode::UNKNOWN, "Unknow error")));
                }
                });
        }
    }
}

