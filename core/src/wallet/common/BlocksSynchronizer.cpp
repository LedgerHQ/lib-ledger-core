#pragma once

#include <mutex>
#include <memory>
#include <vector>

#include <api/ExecutionContext.hpp>
#include <async/FutureUtils.hpp>
#include <wallet/common/BlocksSynchronizer.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>

namespace ledger {
    namespace core {
        namespace common {
            template<typename NetworkType>
            using TransactionBulk = typename ExplorerV2<NetworkType>::TransactionBulk;

            namespace {
                BlockSyncState::BlockSyncState()
                    : _wasFinished(false)
                    , _batchesToSync(0){
                }
                bool BlockSyncState::finishBatch() {
                    std::lock_guard<std::mutex> lock(_lock);
                    if (_wasFinished)
                        return false;
                    _batchesToSync--;
                    bool res = isCompleted();
                    if (res)
                        _wasFinished = true;
                    return res;
                }

                void BlockSyncState::addBatch() {
                    std::lock_guard<std::mutex> lock(_lock);
                    if (_wasFinished)
                        return;
                    _batchesToSync++;
                }

                bool BlockSyncState::isCompleted() {
                    return _batchesToSync == 0;
                }


                BlocksSyncState::BlocksSyncState(uint32_t from, uint32_t to)
                    : fromHeight(from)
                    , _blocks(to - from + 1) {
                }

                bool BlocksSyncState::finishBatch(uint32_t blockHeight) {
                    return _blocks[blockHeight - fromHeight].finishBatch();
                }

                void BlocksSyncState::addBatch(uint32_t from, uint32_t to) {
                    for (uint32_t i = from - fromHeight; i <= to - fromHeight; ++i)
                        _blocks[i].addBatch();
                }

            }

            template<typename NetworkType> BlocksSynchronizer<NetworkType>::BlocksSynchronizer(
                    const std::shared_ptr<api::ExecutionContext> executionContext,
                    const std::shared_ptr<ExplorerV2<NetworkType>>& explorer,
                    const std::shared_ptr<Keychain>& receiveKeychain,
                    const std::shared_ptr<Keychain>& changeKeychain,
                    const std::shared_ptr<BlockchainDatabase<NetworkType>>& blocksDB,
                    uint32_t gapSize,
                    uint32_t batchSize,
                    uint32_t maxTransactionPerResponse)
                : _executionContext(executionContext)
                , _explorer(explorer)
                , _receiveKeychain(receiveKeychain)
                , _changeKeychain(changeKeychain)
                , _blocksDB(blocksDB)
                , _gapSize(gapSize)
                , _batchSize(batchSize)
                , _maxTransactionPerResponse(maxTransactionPerResponse) {
                }

                template<typename NetworkType> Future<Unit> BlocksSynchronizer<NetworkType>::synchronize(
                    const std::string& blockHashToStart,
                    uint32_t firstBlockHeightToInclude,
                    uint32_t lastBlockHeightToInclude) {
                    std::vector<Future<Unit>> tasks; 
                    auto partialBlockDB = std::make_shared<InMemoryPartialBlocksDB<NetworkType>>();
                    auto state = std::make_shared<BlocksSyncState>(firstBlockHeightToInclude, lastBlockHeightToInclude);
                    bootstrapBatchesTasks(state, partialBlockDB, _receiveKeychain, tasks, blockHashToStart, firstBlockHeightToInclude, lastBlockHeightToInclude);
                    bootstrapBatchesTasks(state, partialBlockDB, _changeKeychain, tasks, blockHashToStart, firstBlockHeightToInclude, lastBlockHeightToInclude);
                    return executeAll(_executionContext, tasks).map<Unit>(_executionContext, [](const std::vector<Unit>& vu) { return unit; });
                }

                template<typename NetworkType> Future<Unit> BlocksSynchronizer<NetworkType>::createBatchSyncTask(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& db,
                    const std::shared_ptr<Batch>& batch,
                    const std::shared_ptr<Keychain>& keychain,
                    uint32_t from,
                    uint32_t to,
                    const std::string& firstBlockHash,
                    bool isGap) {
                    if ((batch->addresses.size() == 0) || (to < from)) {
                        return Future<Unit>::successful(unit);
                    }
                    state->addBatch(from, to);
                    return synchronizeBatch(state, db, batch, keychain, from, to, firstBlockHash, isGap);
                }

                template<typename NetworkType> void BlocksSynchronizer<NetworkType>::bootstrapBatchesTasks(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& db,
                    const std::shared_ptr<Keychain>& keychain,
                    std::vector<Future<Unit>>& tasks,
                    std::string firstBlockHash,
                    uint32_t from,
                    uint32_t to) {
                    uint32_t numberOfAddresses = keychain->getNumberOfUsedAddresses();
                    for (uint32_t i = 0; i < numberOfAddresses; i += _batchSize) {
                        auto batch = std::make_shared<Batch>();
                        auto count = std::min(_batchSize, numberOfAddresses - i);
                        batch->lastAddressIndex = i + count - 1;
                        batch->addresses = keychain->getAddresses(i, count);
                        tasks.push_back(createBatchSyncTask(state, db, batch, keychain, from, to, firstBlockHash, false));
                    }
                    // add gapped addresses
                    auto batch = std::make_shared<Batch>();
                    batch->addresses = keychain->getAddresses(numberOfAddresses, _gapSize);
                    tasks.push_back(createBatchSyncTask(state, db, batch, keychain, from, to, firstBlockHash, true));
                }

                template<typename NetworkType> void BlocksSynchronizer<NetworkType>::finilizeBatch(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& partialDB,
                    int from,
                    int to) {
                    if (to < from)
                        return ;
                    for (int i = from; i <= to; ++i) {
                        if (state->finishBatch(i)) {
                            auto trans = partialDB->getTransactions(i);
                            if (trans.size() == 0)
                                continue;
                            FilledBlock block;
                            block.header = Block();
                            block.header.height = i;
                            block.header.hash = trans[0].block.getValue().hash;
                            block.header.createdAt = trans[0].block.getValue().createdAt;
                            block.transactions = trans;
                            _blocksDB->addBlock(block); // write to disk by default is async
                            // try to not consume to much memory
                            partialDB->removeBlock(i);
                        }
                    }
                }

                template<typename NetworkType> Future<Unit> BlocksSynchronizer<NetworkType>::synchronizeBatch(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& partialDB,
                    const std::shared_ptr<Batch>& batch,
                    const std::shared_ptr<Keychain>& keychain,
                    uint32_t from,
                    uint32_t to,
                    const std::string& hashToStartRequestFrom,
                    bool isGap) {
                    auto self = shared_from_this();
                    return
                        _explorer->getTransactions(batch->addresses, hashToStartRequestFrom)
                        .flatMap<Unit>(_executionContext, [self, batch, partialDB, state, keychain, from, to, isGap](const TransactionBulk<NetworkType>& bulk) {
                        if (bulk.first.size() == 0) {
                            self->finilizeBatch(state, partialDB, from, to);
                            return Future<Unit>::successful(unit);
                        }
                        static std::function<bool(const Transaction& l, const Transaction& r)> blockLess = [](const Transaction& l, const Transaction& r)->bool {return l.block.getValue().height < r.block.getValue().height; };
                        Block highestBlock = std::max_element(bulk.first.begin(), bulk.first.end(), blockLess)->block.getValue();
                        Block lowestBlock = std::min_element(bulk.first.begin(), bulk.first.end(), blockLess)->block.getValue();
                        uint32_t lastFullBlockHeight = to;
                        if (bulk.second) { //response truncated
                            if (bulk.first.size() < self->_maxTransactionPerResponse) {
                                // TODO: log warning and notify Explorer team to increase threshold
                            }
                            if (highestBlock.height != from) { // Explorer garanties us that we always get at least one full block
                                lastFullBlockHeight = highestBlock.height - 1;
                            }
                            else {
                                throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "The case with only one block transactions is not supported.");
                            }
                        }
                        uint32_t limit = std::min(lastFullBlockHeight, to); // want only transactions from untruncated blocks
                        for (auto &tr : bulk.first) {
                            if (tr.block.getValue().height <= limit) {
                                partialDB->addTransaction(tr);
                                for (auto& out : tr.outputs) {
                                    if (!out.address.hasValue()) continue;
                                    keychain->markAsUsed(out.address.getValue());
                                }
                            }
                        }
                        std::vector<Future<Unit>> tasksToContinueWith;
                        if (isGap) {
                            auto newBatch = std::make_shared<Batch>();
                            newBatch->addresses = keychain->getAddresses(batch->lastAddressIndex + 1, self->_batchSize);
                            newBatch->lastAddressIndex = batch->lastAddressIndex + self->_batchSize;
                            tasksToContinueWith.push_back(self->createBatchSyncTask(state, partialDB, newBatch, keychain, lowestBlock.height, to, lowestBlock.hash, true));
                        }
                        tasksToContinueWith.push_back(self->createBatchSyncTask(state, partialDB, batch, keychain, lastFullBlockHeight + 1, to, highestBlock.hash, false));
                        self->finilizeBatch(state, partialDB, from, to);
                        return executeAll(self->_executionContext, tasksToContinueWith).map<Unit>(self->_executionContext, [](const std::vector<Unit>& vu) { return unit; });
                    });
                }
        }
    }
}

template class ledger::core::common::BlocksSynchronizer<ledger::core::BitcoinLikeNetwork>;
template class std::shared_ptr<ledger::core::common::BlocksSynchronizer<ledger::core::BitcoinLikeNetwork>>;