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
                std::shared_ptr<core::AccountSynchronizer<BitcoinLikeNetwork>> createAccountSynchronizer(
                    const std::shared_ptr<api::ExecutionContext>& executionContext,
                    const std::shared_ptr<ExplorerV2<BitcoinLikeNetwork>>& explorer,
                    const std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>>& stableBlocksDb,
                    const std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>>& unstableBlocksDb,
                    const std::shared_ptr<Keychain>& keychain,
                    const std::shared_ptr<spdlog::logger>& logger,
                    uint32_t numberOfUnrevertableBlocks,
                    uint32_t maxNumberOfAddressesInRequest,
                    uint32_t discoveryGapSize
                ) override;
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<ExplorerV2<BitcoinLikeNetwork>> _explorer;
            };
        }
    }
}
