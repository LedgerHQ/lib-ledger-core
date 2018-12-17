#include <wallet/common/AccountSynchronizer.hpp>
#include <spdlog/spdlog.h>
#include <events/ProgressNotifier.h>
#include <api/ExecutionContext.hpp>
#include <async/DedicatedContext.hpp>
#include <preferences/Preferences.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/common/BlocksSynchronizer.hpp>
#include <async/FutureUtils.hpp>

namespace ledger {
    namespace core {
        namespace common {
            template <typename NetworkType> AccountSynchronizer<NetworkType>::AccountSynchronizer(
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<ExplorerV2<NetworkType>>& explorer,
                const std::shared_ptr<BlockchainDatabase<NetworkType>>& stableBlocksDb,
                const std::shared_ptr<BlockchainDatabase<NetworkType>>& unstableBlocksDb,
                const std::shared_ptr<BlockchainDatabase<NetworkType>>& pendingTransactionsDb,
                const std::shared_ptr<Keychain>& receiveKeychain,
                const std::shared_ptr<Keychain>& changeKeychain,
                const std::shared_ptr<spdlog::logger>& logger,
                const SynchronizerConfiguration& synchronizerConfig
            )
                : _executionContext(executionContext)
                , _explorer(explorer)
                , _stableBlocksDb(stableBlocksDb)
                , _unstableBlocksDb(unstableBlocksDb)
                , _pendingTransactionsDb(pendingTransactionsDb)
                , _receiveKeychain(receiveKeychain)
                , _changeKeychain(changeKeychain)
                , _logger(logger)
                , _config(synchronizerConfig) {
                _stableBlocksSynchronizer = std::make_shared<BlocksSynchronizer<NetworkType>>(
                    _executionContext,
                    _explorer,
                    _receiveKeychain,
                    _changeKeychain,
                    _stableBlocksDb,
                    _config.discoveryGapSize,
                    _config.maxNumberOfAddressesInRequest,
                    _config.maxTransactionPerResponce);

                _unstableBlocksSynchronizer = std::make_shared<BlocksSynchronizer<NetworkType>>(
                    _executionContext,
                    _explorer,
                    _receiveKeychain,
                    _changeKeychain,
                    _unstableBlocksDb,
                    _config.discoveryGapSize,
                    _config.maxNumberOfAddressesInRequest,
                    _config.maxTransactionPerResponce);
            }

            template <typename NetworkType> std::shared_ptr<ProgressNotifier<Unit>> AccountSynchronizer<NetworkType>::synchronize() {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                if (_notifier != nullptr) {
                    // we are currently in synchronization
                    return _notifier;
                }
                _notifier = std::make_shared<ProgressNotifier<Unit>>();
                auto self = this->shared_from_this();
                self->synchronizeBlocks()
                    .template flatMap<Unit>(_executionContext, [self](const Unit& dummy) { return self->synchronizePendingTransactions(); })
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

            template <typename NetworkType> bool AccountSynchronizer<NetworkType>::isSynchronizing() const {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                return _notifier != nullptr;
            }

            template <typename NetworkType> Future<Unit> AccountSynchronizer<NetworkType>::synchronizeBlocks() {
                auto self = this->shared_from_this();
                // try to do explorer and DB request simulteniously
                Future<HashHeight> explorerRequest = _explorer->getCurrentBlock().template map<HashHeight>(_executionContext, [](const Block& block) { return HashHeight{ block.hash, block.height }; });
                auto createLastHHFuture = [blockDB = _stableBlocksDb, executionContext = this->_executionContext, startHash = _config.genesisBlockHash]() {
                    return blockDB->getLastBlockHeader()
                        .template map<HashHeight>(
                            executionContext,
                            [startHash](const Option<Block>& block) {
                                HashHeight hh{ startHash, 0 };
                                if (block.hasValue()) {
                                    hh.height = block.getValue().height;
                                    hh.hash = block.getValue().hash;
                                }
                                return hh; });
                };
                auto futures = std::vector<Future<HashHeight>>{explorerRequest, createLastHHFuture()};
                return
                    executeAll(_executionContext, futures)
                    .template flatMap<Unit>(_executionContext, [self, createLastHHFuture](const std::vector<HashHeight>& hhs) {
                        return self->_stableBlocksSynchronizer->synchronize(
                            hhs[1].hash,
                            hhs[1].height,
                            hhs[0].height - self->_config.maxPossibleUnstableBlocks)
                            .template flatMap<HashHeight>(self->_executionContext, [createLastHHFuture](const Unit& dummy) {return createLastHHFuture(); })
                            .template flatMap<Unit>(self->_executionContext, [self, currentBlockHeight=hhs[0].height](const HashHeight& stableHH) {
                            self->_unstableBlocksDb->CleanAll();
                            return self->_unstableBlocksSynchronizer->synchronize(stableHH.hash, currentBlockHeight - self->_config.maxPossibleUnstableBlocks + 1, currentBlockHeight);
                        });
                        });
            }

            template <typename NetworkType> Future<Unit> AccountSynchronizer<NetworkType>::synchronizePendingTransactions() {
                //TODO: Implement this function for ExplorerV3
                return Future<Unit>::successful(unit);
            }
        }
    }
}

template class ledger::core::common::AccountSynchronizer<ledger::core::BitcoinLikeNetwork>;
template class std::shared_ptr<ledger::core::common::AccountSynchronizer<ledger::core::BitcoinLikeNetwork>>;
