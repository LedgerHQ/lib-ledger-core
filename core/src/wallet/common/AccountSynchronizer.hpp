#pragma once
#include <spdlog/spdlog.h>

#include <api/ExecutionContext.hpp>
#include <async/DedicatedContext.hpp>
#include <database/PartialBlocksDB.hpp>
#include <preferences/Preferences.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <wallet/AccountSynchronizer.hpp>
#include <async/FutureUtils.hpp>

namespace ledger {
    namespace core {
        namespace common {
            namespace {
                class BlockchainState {
                public:
                    std::shared_ptr<Block> currentBlock;
                    std::shared_ptr<Block> lastStableBlock;
                    std::shared_ptr<Block> lastUnstableBlock;
                };
            };

            template<typename NetworkType>
            class AccountSynchronizer :
                public core::AccountSynchronizer<NetworkType>,
                public std::enable_shared_from_this<AccountSynchronizer<NetworkType>> {
            public:
                typedef ExplorerV2<NetworkType> Explorer;
                typedef BlockchainDatabase<NetworkType> BlockchainDB;

            AccountSynchronizer(
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<Explorer>& explorer,
                const std::shared_ptr<BlockchainDB>& stableBlocksDb,
                const std::shared_ptr<BlockchainDB>& unstableBlocksDb,
                const std::shared_ptr<Keychain<NetworkType>>& keychain,
                const std::shared_ptr<spdlog::logger>& logger,
                uint32_t maxPossibleUnstableBlocks,
                uint32_t maxNumberOfAddressesInRequest,
                uint32_t discoveryGapSize
            )
                : _executionContext(executionContext)
                , _explorer(explorer)
                , _stableBlocksDb(stableBlocksDb)
                , _unstableBlocksDb(unstableBlocksDb)
                , _keychain(keychain)
                , _maxPossibleUnstableBlocks(maxPossibleUnstableBlocks)
                , _maxNumberOfAddressesInRequest(maxNumberOfAddressesInRequest)
                , _discoveryGapSize(discoveryGapSize)
                , _logger(logger) {
            }

            std::shared_ptr<ProgressNotifier<Unit>> synchronize() override {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                if (_notifier != nullptr) {
                    // we are currently in synchronization
                    return _notifier;
                }
                _notifier = std::make_shared<ProgressNotifier<Unit>>();
                auto self = shared_from_this();
                getState()
                    .flatMap<Unit>(_executionContext, [self](const std::shared_ptr<BlockchainState>& state) { return self->synchronizeBlocks(state); })
                    .flatMap<Unit>(_executionContext, [self](const Unit& dummy) { return self->synchronizePendingTransactions(); })
                    .onComplete(_executionContext, [self](const Try<Unit> &result) {
                        std::lock_guard<std::recursive_mutex> l(self->_lock);
                        if (result.isFailure()) {
                            self->_notifier->failure(result.getFailure());
                        }
                        else {
                            self->_notifier->success(unit);
                        }
                        self->_notifier = nullptr;
                    });

                return _notifier;
            }

            Future<std::shared_ptr<BlockchainState>> getState() {
                std::vector<FuturePtr<Block>> blocksFuture { 
                    _explorer->getCurrentBlock(),
                    _stableBlocksDb->getLastBlockHeader(),
                    _unstableBlocksDb->getLastBlockHeader()};
                return executeAll(_executionContext, blocksFuture).map<std::shared_ptr<BlockchainState>>(_executionContext, [](const std::vector<std::shared_ptr<Block>> input) {
                    auto state = std::make_shared<BlockchainState>();
                    state->currentBlock = input[0];
                    state->lastStableBlock = input[1];
                    state->lastUnstableBlock = input[2];
                    return state;
                });
            }

            bool isSynchronizing() const {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                return _notifier != nullptr;
            }

            Future<Unit> synchronizeBlocks(const std::shared_ptr<BlockchainState>& state) {
                if (state->currentBlock == state->lastStableBlock)
                    return Future<Unit>::successful(unit);
                
            }

            Future<Unit> synchronizePendingTransactions() {
                return Future<Unit>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me"));
            }

            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<Explorer> _explorer;
                // blocks that would not be reverted
                std::shared_ptr<BlockchainDB> _stableBlocksDb;
                // blocks that may be reverted
                std::shared_ptr<BlockchainDB> _unstableBlocksDb;
                std::shared_ptr<Keychain<NetworkType>> _keychain;
                std::shared_ptr<spdlog::logger> _logger;

                std::shared_ptr<ProgressNotifier<Unit>> _notifier;
                const uint32_t _maxPossibleUnstableBlocks;
                const uint32_t _maxNumberOfAddressesInRequest;
                const uint32_t _discoveryGapSize;
                mutable std::recursive_mutex _lock; // recursive in case somebody will use ImmediateExecutionContext 
            };
        }
    }
}
