#pragma once

#include <wallet/AccountSynchronizerFactory.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class AccountSynchronizerFactory : public core::AccountSynchronizerFactory<BitcoinLikeNetwork> {
            public:
                AccountSynchronizerFactory(
                    std::shared_ptr<api::ExecutionContext> executionContext,
                    std::shared_ptr<ExplorerV2<BitcoinLikeNetwork>> explorer);
                std::shared_ptr<core::AccountSynchronizer> createAccountSynchronizer(
                    std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>> stableBlocksDb,
                    std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>> unstableBlocksDb
                ) override;
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<ExplorerV2<BitcoinLikeNetwork>> _explorer;
            };
        }
    }
}
