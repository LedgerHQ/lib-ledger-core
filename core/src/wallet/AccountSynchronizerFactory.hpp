#pragma once

#include <memory>
#include <wallet/AccountSynchronizer.hpp>
#include <wallet/BlockchainDatabase.hpp>

namespace spdlog {
    class logger;
}

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class ExplorerV2;

        template<typename NetworkType>
        class Keychain;

        template<typename NetworkType>
        class AccountSynchronizerFactory {
        public:
            virtual std::shared_ptr<AccountSynchronizer<NetworkType>> createAccountSynchronizer(
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<ExplorerV2<NetworkType>>& explorer,
                const std::shared_ptr<BlockchainDatabase<NetworkType>>& stableBlocksDb,
                const std::shared_ptr<BlockchainDatabase<NetworkType>>& unstableBlocksDb,
                const std::shared_ptr<Keychain<NetworkType>>& keychain,
                const std::shared_ptr<spdlog::logger>& logger,
                uint32_t numberOfUnrevertableBlocks,
                uint32_t maxNumberOfAddressesInRequest,
                uint32_t discoveryGapSize) = 0;
            virtual ~AccountSynchronizerFactory() {};
        };
    }
}
