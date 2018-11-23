#pragma once

#include <memory>
#include <wallet/bitcoin/factories/AccountSynchronizerFactory.hpp>
#include <wallet/bitcoin/synchronizers/AccountSynchronizer.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            AccountSynchronizerFactory::AccountSynchronizerFactory(
                std::shared_ptr<api::ExecutionContext> executionContext,
                std::shared_ptr<ExplorerV2<BitcoinLikeNetwork>> explorer)
            : _executionContext(executionContext)
            , _explorer(explorer) {

            }

            std::shared_ptr<core::AccountSynchronizer> AccountSynchronizerFactory::createAccountSynchronizer(
                std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>> stableBlocksDb,
                std::shared_ptr<BlockchainDatabase<BitcoinLikeNetwork>> unstableBlocksDb) {
                return std::make_shared<AccountSynchronizer>(
                    _executionContext,
                    _explorer,
                    stableBlocksDb,
                    unstableBlocksDb
                    );
            };
        }
    }
}
