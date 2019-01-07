#include <api/ExecutionContext.hpp>
#include <async/DedicatedContext.hpp>
#include <async/FutureUtils.hpp>
#include <events/ProgressNotifier.h>
#include <preferences/Preferences.hpp>
#include <spdlog/spdlog.h>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/StateManager.hpp>
#include <wallet/common/AccountSynchronizer.hpp>
#include <wallet/common/BlocksSynchronizer.hpp>
#include <wallet/common/InMemoryBlockchainDatabase.hpp>
#include <wallet/common/BlockchainDatabaseView.hpp>

namespace ledger {
    namespace core {
        namespace common {

            template<typename FilledBlock>
            using DBPair = typename std::pair<std::shared_ptr<ReadOnlyBlockchainDatabase<FilledBlock>>, std::shared_ptr<ReadOnlyBlockchainDatabase<FilledBlock>>>;
            

            template <typename NetworkType>
            AccountSynchronizer<NetworkType>::AccountSynchronizer(
                const std::shared_ptr<StateManager<FilledBlock>>& stateManager,
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<ExplorerV2<NetworkType>>& explorer,
                const std::shared_ptr<BlocksDatabase>& stableBlocksDb,
                const std::shared_ptr<Keychain>& receiveKeychain,
                const std::shared_ptr<Keychain>& changeKeychain,
                const std::shared_ptr<spdlog::logger>& logger,
                const SynchronizerConfiguration& synchronizerConfig
            )
                : _stateManager(stateManager)
                , _executionContext(executionContext)
                , _explorer(explorer)
                , _stableBlocksDb(stableBlocksDb)
                , _receiveKeychain(receiveKeychain)
                , _changeKeychain(changeKeychain)
                , _logger(logger)
                , _config(synchronizerConfig) {
                _blocksSynchronizer = std::make_shared<BlocksSynchronizer<NetworkType>>(
                    _executionContext,
                    _explorer,
                    _receiveKeychain,
                    _changeKeychain,
                    _config.discoveryGapSize,
                    _config.maxNumberOfAddressesInRequest,
                    _config.maxTransactionPerResponce);
            }

            template <typename NetworkType>
            std::shared_ptr<ProgressNotifier<Unit>> AccountSynchronizer<NetworkType>::synchronize() {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                if (_notifier != nullptr) {
                    // we are currently in synchronization
                    return _notifier;
                }
                _notifier = std::make_shared<ProgressNotifier<Unit>>(_executionContext);
                auto self = this->shared_from_this();
                synchronizeBlocks()
                    .template flatMap<BlockchainState<FilledBlock>>(_executionContext, [self](const DBPair<FilledBlock>& dbs) {
                        return 
                            self->synchronizePendingTransactions()
                            .template map<BlockchainState<FilledBlock>>(self->_executionContext, [self, dbs](const std::vector<Transaction>& transactions) {
                                BlockchainState<FilledBlock> res(dbs.first, dbs.second, transactions);
                                return res;
                            });
                    })
                    .onComplete(_executionContext, [self](const Try<BlockchainState<FilledBlock>> &result) {
                        std::lock_guard<std::recursive_mutex> l(self->_lock);
                        if (result.isFailure()) {
                            self->_notifier->failure(result.getFailure());
                        }
                        else {
                            self->_stateManager->setState(result.getValue());
                            self->_notifier->success(unit);
                        }
                        self->_notifier = nullptr;
                    });

                return _notifier;
            }

            template <typename NetworkType>
            bool AccountSynchronizer<NetworkType>::isSynchronizing() const {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                return _notifier != nullptr;
            }

            template <typename NetworkType> 
            Future<DBPair<typename NetworkType::FilledBlock>> AccountSynchronizer<NetworkType>::synchronizeBlocks() {
                auto self = this->shared_from_this();
                // try to do explorer and DB request simulteniously
                Future<HashHeight> explorerRequest = _explorer->getCurrentBlock().template map<HashHeight>(_executionContext, [](const Block& block) { return HashHeight{ block.hash, block.height }; });
                auto createLastHHFuture = [blockDB = _stableBlocksDb, executionContext = this->_executionContext, startHash = _config.genesisBlockHash]() {
                    return blockDB->getLastBlock()
                        .template map<HashHeight>(
                            executionContext,
                            [startHash](const Option<std::pair<uint32_t, FilledBlock>>& block) {
                                HashHeight hh{ startHash, 0 };
                                if (block.hasValue()) {
                                    hh.height = block.getValue().first;
                                    hh.hash = block.getValue().second.header.hash;
                                }
                                return hh; });
                };
                auto futures = std::vector<Future<HashHeight>>{explorerRequest, createLastHHFuture()};
                return
                    executeAll(_executionContext, futures)
                    .template flatMap<DBPair<FilledBlock>>(_executionContext, [self, createLastHHFuture](const std::vector<HashHeight>& hhs) {
                        return self->_blocksSynchronizer->synchronize(
                            self->_stableBlocksDb,
                            hhs[1].hash,
                            hhs[1].height,
                            hhs[0].height - self->_config.maxPossibleUnstableBlocks)
                            .template flatMap<HashHeight>(self->_executionContext, [createLastHHFuture](const Unit& dummy) {return createLastHHFuture(); })
                            .template flatMap<DBPair<FilledBlock>>(self->_executionContext, [self, currentBlockHeight=hhs[0].height](const HashHeight& stableHH) {
                            auto unstableDb = std::make_shared<InMemoryBlockchainDatabase<FilledBlock>>(self->_executionContext);
                            return self->_blocksSynchronizer->synchronize(unstableDb, stableHH.hash, currentBlockHeight - self->_config.maxPossibleUnstableBlocks + 1, currentBlockHeight)
                                .template map<DBPair<FilledBlock>>(self->_executionContext, [self, unstableDb, stableHH](const Unit& dummy) {
                                    return DBPair<FilledBlock>(
                                        std::make_shared<BlockchainDatabaseView<FilledBlock>>(stableHH.height, self->_stableBlocksDb),
                                        unstableDb
                                        );
                                });
                        });
                        });
            }

            template <typename NetworkType>
            Future<std::vector<typename NetworkType::Transaction>> AccountSynchronizer<NetworkType>::synchronizePendingTransactions() {
                //TODO: Implement this function for ExplorerV3
                std::vector<Transaction> res;
                return Future<std::vector<Transaction>>::successful(res);
            }
        }
    }
}

template class ledger::core::common::AccountSynchronizer<ledger::core::BitcoinLikeNetwork>;
template class std::shared_ptr<ledger::core::common::AccountSynchronizer<ledger::core::BitcoinLikeNetwork>>;
