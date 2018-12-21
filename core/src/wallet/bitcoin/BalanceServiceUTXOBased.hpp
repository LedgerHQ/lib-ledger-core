#pragma once

#include <memory>
#include <wallet/BalanceService.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class UTXOService;

            class BalanceServiceUTXOBased : public BalanceService {
            public:
                BalanceServiceUTXOBased(const std::shared_ptr<api::ExecutionContext>& executionContext, const std::shared_ptr<UTXOService>& utxoService);
                Future<BigInt> getBalance() override;
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<UTXOService> _utxoService;
            };
        }
    }
}