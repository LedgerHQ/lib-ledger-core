#pragma once

#include <api/ExecutionContext.hpp>
#include <async/DedicatedContext.hpp>
#include <database/PartialBlocksDB.hpp>
#include <preferences/Preferences.hpp>
#include <wallet/BlockainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <wallet/AccountSynchronizer.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            // State of the world before synchronization
            struct BlockchainState {
                BitcoinLikeNetwork::Block currentBlock;
                BitcoinLikeNetwork::Block lastStableBlock;
                BitcoinLikeNetwork::Block lastUnstableBlock;
            };

            class BitcoinLikeAccountSynchronizer :
                public AccountSynchronizer,
                public std::enable_shared_from_this<BitcoinLikeAccountSynchronizer> {
            public:
                typedef ExplorerV2<BitcoinLikeNetwork> Explorer;
                typedef BlockchainDatabase<BitcoinLikeNetwork> BlockchainDB;

                BitcoinLikeAccountSynchronizer(
                    const std::shared_ptr<api::ExecutionContext>& executionContext,
                    const std::shared_ptr<Explorer>& explorer,
                    const std::shared_ptr<BlockchainDB>& stableBlocksDb,
                    const std::shared_ptr<BlockchainDB>& unstableBlocksDb
                );

                std::shared_ptr<ProgressNotifier<Unit>> synchronize() override;

                bool isSynchronizing() const;
            private:
                Future<BlockchainState> getState();
                Future<Unit> synchronizeStable(int fromHeight, int toHeight);
                Future<Unit> synchronizeUnstable(int fromHeight, int toHeight);
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<Explorer> _explorer;
                // blocks that would not be reverted
                std::shared_ptr<BlockchainDB> _stableBlocksDb;
                // blocks that may be reverted
                std::shared_ptr<BlockchainDB> _unstableBlocksDb;
                std::shared_ptr<ProgressNotifier<Unit>> _notifier;
                
                mutable std::mutex _lock;
            };
        }
    }
}
