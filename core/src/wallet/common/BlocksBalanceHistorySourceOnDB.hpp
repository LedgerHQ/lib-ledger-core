#pragma once
#include <api/ExecutionContext.hpp>
#include <wallet/BlocksBalanceHistorySource.hpp>
#include <wallet/Keychain.hpp>

namespace ledger {
    namespace core {
        namespace common {

            class BlocksBalanceHistorySourceOnDB : public BlocksBalanceHistorySource {
            public:
                BlocksBalanceHistorySourceOnDB(
                    const std::shared_ptr<api::ExecutionContext>& executionContext,
                    const std::shared_ptr<KeychainRegistry>& addressRegistry);
                std::map<uint32_t, BigInt> getHistory() override;
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<KeychainRegistry>
            };
        }
    }
}