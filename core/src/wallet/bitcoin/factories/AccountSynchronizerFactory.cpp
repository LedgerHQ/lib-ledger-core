#pragma once

#include <memory>
#include <wallet/bitcoin/factories/AccountSynchronizerFactory.hpp>
#include <wallet/common/AccountSynchronizer.hpp>
#include <wallet/Keychain.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            AccountSynchronizerFactory::AccountSynchronizerFactory(
                std::shared_ptr<api::ExecutionContext> executionContext,
                std::shared_ptr<ExplorerV2<BitcoinLikeNetwork>> explorer)
            : _executionContext(executionContext)
            , _explorer(explorer) {

            }

            std::shared_ptr<core::AccountSynchronizer<BitcoinLikeNetwork>> AccountSynchronizerFactory::createAccountSynchronizer(
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<ExplorerV2<BitcoinLikeNetwork>>& explorer,
                const std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>>& stableBlocksDb,
                const std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>>& unstableBlocksDb,
                const std::shared_ptr<Keychain>& keychain,
                const std::shared_ptr<spdlog::logger>& logger,
                uint32_t numberOfUnrevertableBlocks,
                uint32_t maxNumberOfAddressesInRequest,
                uint32_t discoveryGapSize) {
                return std::make_shared<common::AccountSynchronizer<BitcoinLikeNetwork>>(
                    _executionContext,
                    _explorer,
                    stableBlocksDb,
                    unstableBlocksDb,
                    keychain,
                    logger,
                    numberOfUnrevertableBlocks,
                    maxNumberOfAddressesInRequest,
                    discoveryGapSize);
            };
        }
    }
}
