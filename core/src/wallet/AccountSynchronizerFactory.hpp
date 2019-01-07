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

        class Keychain;

        template<typename NetworkType>
        class AccountSynchronizerFactory {
        public:
            typedef typename NetworkType::FilledBlock FilledBlock;
            typedef BlockchainDatabase<FilledBlock> BlocksDatabase;
        public:
            virtual std::shared_ptr<core::AccountSynchronizer<NetworkType>> createAccountSynchronizer(
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<ExplorerV2<NetworkType>>& explorer,
                const std::shared_ptr<BlocksDatabase>& stableBlocksDb,
                const std::shared_ptr<Keychain>& receiveKeychain,
                const std::shared_ptr<Keychain>& changeKeychain,
                const std::shared_ptr<spdlog::logger>& logger,
                uint32_t numberOfUnrevertableBlocks,
                uint32_t maxNumberOfAddressesInRequest,
                uint32_t discoveryGapSize) = 0;
            virtual ~AccountSynchronizerFactory() {};
        };
    }
}
