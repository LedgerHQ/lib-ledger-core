#pragma once

#include <wallet/bitcoin/UTXOService.hpp>

namespace ledger {
    namespace core {
        namespace api {
            class ExecutionContext;
        };
        namespace bitcoin {
            class UTXOSource;
            /// Implementation of UTXOService interface based on UTXOSources
            class UTXOServiceSourceBased : public UTXOService, public std::enable_shared_from_this<UTXOServiceSourceBased> {
            public:
                UTXOServiceSourceBased(
                    const std::shared_ptr<api::ExecutionContext>& executionContext,
                    const std::shared_ptr<UTXOSource>& stableBlocksSource,
                    const std::shared_ptr<UTXOSource>& unstableBlocksSource,
                    const std::shared_ptr<UTXOSource>& pendingTransactionsSource);
                Future<std::map<UTXOKey, UTXOValue>> getUTXOs() override;
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<UTXOSource> _stableBlocksSource;
                std::shared_ptr<UTXOSource> _unstableBlocksSource;
                std::shared_ptr<UTXOSource> _pendingTransactionsSource;
            };
        }
    }
}