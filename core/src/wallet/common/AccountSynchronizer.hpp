#pragma once
#include <spdlog/spdlog.h>

#include <api/ExecutionContext.hpp>
#include <async/DedicatedContext.hpp>
#include <preferences/Preferences.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/AccountSynchronizer.hpp>
#include <async/FutureUtils.hpp>

namespace ledger {
    namespace core {
        namespace common {
            namespace {
                template<typename BlockType>
                class BlockchainState {
                public:
                    BlockType currentBlock;
                    BlockType lastStableBlock;
                    BlockType lastUnstableBlock;
                };
            };

            template<typename NetworkType>
            class AccountSynchronizer :
                public core::AccountSynchronizer<NetworkType>,
                public std::enable_shared_from_this<AccountSynchronizer<NetworkType>> {
            public:
                typedef typename NetworkType::Block Block;
                typedef ExplorerV2<NetworkType> Explorer;
                typedef BlockchainDatabase<NetworkType> BlockchainDatabase;

            AccountSynchronizer(
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<Explorer>& explorer,
                const std::shared_ptr<BlockchainDatabase>& stableBlocksDb,
                const std::shared_ptr<BlockchainDatabase>& unstableBlocksDb,
                const std::shared_ptr<Keychain>& keychain,
                const std::shared_ptr<spdlog::logger>& logger,
                uint32_t maxPossibleUnstableBlocks,
                uint32_t maxNumberOfAddressesInRequest,
                uint32_t discoveryGapSize,
                const Block& genesisBlock
            )
                : _executionContext(executionContext)
                , _explorer(explorer)
                , _stableBlocksDb(stableBlocksDb)
                , _unstableBlocksDb(unstableBlocksDb)
                , _keychain(keychain)
                , _maxPossibleUnstableBlocks(maxPossibleUnstableBlocks)
                , _maxNumberOfAddressesInRequest(maxNumberOfAddressesInRequest)
                , _discoveryGapSize(discoveryGapSize)
                , _logger(logger)
                , _genesisBlock(genesisBlock) {
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
                    .flatMap<Unit>(_executionContext, [self](const BlockchainState<Block>& state) { return self->synchronizeBlocks(state); })
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

            Future<BlockchainState<Block>> getState() {
                std::vector<Future<Block>> blocksFuture { 
                    _explorer->getCurrentBlock(),
                    Future<Block>::mapValueOr(_stableBlocksDb->getLastBlockHeader(), _genesisBlock),
                    Future<Block>::mapValueOr(_unstableBlocksDb->getLastBlockHeader(), _genesisBlock) };
                return executeAll(_executionContext, blocksFuture).map<BlockchainState<Block>>(_executionContext, [](const std::vector<Block> input) {
                    BlockchainState<Block> state;
                    state.currentBlock = input[0];
                    state.lastStableBlock = input[1];
                    state.lastUnstableBlock = input[2];
                    return state;
                });
            }

            bool isSynchronizing() const {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                return _notifier != nullptr;
            }

            Future<Unit> synchronizeBlocks(const BlockchainState<Block>& state) {
                if (state.currentBlock.height == state.lastStableBlock.height)
                    return Future<Unit>::successful(unit);
                return Future<Unit>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me"));
            }

            Future<Unit> synchronizePendingTransactions() {
                return Future<Unit>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me"));
            }

            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<Explorer> _explorer;
                // blocks that would not be reverted
                std::shared_ptr<BlockchainDatabase> _stableBlocksDb;
                // blocks that may be reverted
                std::shared_ptr<BlockchainDatabase> _unstableBlocksDb;
                std::shared_ptr<Keychain> _keychain;
                std::shared_ptr<spdlog::logger> _logger;
                const Block _genesisBlock;

                std::shared_ptr<ProgressNotifier<Unit>> _notifier;
                const uint32_t _maxPossibleUnstableBlocks;
                const uint32_t _maxNumberOfAddressesInRequest;
                const uint32_t _discoveryGapSize;
                mutable std::recursive_mutex _lock; // recursive in case somebody will use ImmediateExecutionContext 
            };
        }
    }
}
