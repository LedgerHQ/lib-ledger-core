#pragma once

#include <mutex>
#include <memory>
#include <vector>

#include <api/ExecutionContext.hpp>
#include <async/FutureUtils.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>

namespace ledger {
    namespace core {
        namespace common {
            namespace {

                struct Batch {
                    std::vector<std::string> addresses;
                    uint32_t lastAddressIndex;
                };

                class BlockSyncState {
                public:
                    BlockSyncState()
                        : _wasFinished(false)
                        , _batchesToSync(0){
                    }
                    bool finishBatch() {
                        std::lock_guard<std::mutex> lock(_lock);
                        if (_wasFinished)
                            return false;
                        _batchesToSync--;
                        bool res = isCompleted();
                        if (res)
                            _wasFinished = true;
                        return res;
                    }

                    void addBatch() {
                        std::lock_guard<std::mutex> lock(_lock);
                        if (_wasFinished)
                            return;
                        _batchesToSync++;
                    }
                private:
                    bool isCompleted() {
                        return _batchesToSync == 0;
                    }
                    std::mutex _lock;
                    uint32_t _batchesToSync;
                    bool _wasFinished;
                };

                class BlocksSyncState {
                public:
                    BlocksSyncState(uint32_t from, uint32_t to)
                        : fromHeight(from)
                        , _blocks(to - from + 1) {
                    }

                    bool finishBatch(uint32_t blockHeight) {
                        return _blocks[blockHeight - fromHeight].finishBatch();
                    }

                    void addBatch(uint32_t from, uint32_t to) {
                        for (uint32_t i = from - fromHeight; i <= to - fromHeight; ++i)
                            _blocks[i].addBatch();
                    }

                private:
                    std::vector<BlockSyncState> _blocks;
                private:
                    const uint32_t fromHeight;
                };
            }

            template<typename NetworkType>
            class BlocksSynchronizer : public std::enable_shared_from_this<BlocksSynchronizer<NetworkType>>{
            public:
                typedef ExplorerV2<NetworkType> Explorer;
                typedef typename Explorer::TransactionBulk TransactionBulk;
                typedef typename NetworkType::Block Block;
                typedef typename NetworkType::Transaction Transaction;
                typedef typename NetworkType::FilledBlock FilledBlock;

                BlocksSynchronizer(
                    const std::shared_ptr<api::ExecutionContext> executionContext,
                    const std::shared_ptr<Explorer>& explorer,
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

                Future<Unit> synchronize(
                    const std::shared_ptr<Block>& firstBlock,
                    const std::shared_ptr<Block>& lastBlock) {
                    std::vector<Future<Unit>> tasks; 
                    auto partialBlockDB = std::make_shared<InMemoryPartialBlocksDB<NetworkType>>();
                    auto state = std::make_shared<BlocksSyncState>(firstBlock->height, lastBlock->height);
                    bootstrapBatchesTasks(state, partialBlockDB, _receiveKeychain, tasks, firstBlock->hash, firstBlock->height, lastBlock->height);
                    bootstrapBatchesTasks(state, partialBlockDB, _changeKeychain, tasks, firstBlock->hash, firstBlock->height, lastBlock->height);
                    return executeAll(_executionContext, tasks).map<Unit>(_executionContext, [](const std::vector<Unit>& vu) { return unit; });
                }
            private:

                Future<Unit> createBatchSyncTask(
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

                void bootstrapBatchesTasks(
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

                void finilizeBatch(
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
                            block.first = Block();
                            block.first.height = i;
                            block.first.hash = trans[0].block.getValue().hash;
                            block.first.createdAt = trans[0].block.getValue().createdAt;
                            block.second = trans;
                            _blocksDB->addBlock(block); // write to disk by default is async
                            // try to not consume to much memory
                            partialDB->removeBlock(i);
                        }
                    }
                }

                Future<Unit> synchronizeBatch(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& partialDB,
                    const std::shared_ptr<Batch>& batch,
                    const std::shared_ptr<Keychain>& keychain,
                    uint32_t from,
                    uint32_t to,
                    std::string hashToStartRequestFrom,
                    bool isGap) {
                    auto self = shared_from_this();
                    return
                        _explorer->getTransactions(batch->addresses, hashToStartRequestFrom)
                        .flatMap<Unit>(_executionContext, [self, batch, partialDB, state, keychain, from, to, isGap](const std::shared_ptr<TransactionBulk>& bulk) {
                        if (bulk->first.size() == 0) {
                            self->finilizeBatch(state, partialDB, from, to);
                            return Future<Unit>::successful(unit);
                        }
                        static std::function<bool(const Transaction& l, const Transaction& r)> blockLess = [](const Transaction& l, const Transaction& r)->bool {return l.block.getValue().height < r.block.getValue().height; };
                        Block highestBlock = std::max_element(bulk->first.begin(), bulk->first.end(), blockLess)->block.getValue();
                        Block lowestBlock = std::min_element(bulk->first.begin(), bulk->first.end(), blockLess)->block.getValue();
                        if (lowestBlock.height < from) {
                            return Future<Unit>::failure(Exception(api::ErrorCode::API_ERROR, "Explorer return transaction with block height less then requested"));
                        }
                        uint32_t lastFullBlockHeight = to;
                        if (bulk->second) { //response truncated
                            if (bulk->first.size() < self->_maxTransactionPerResponse) {
                                return Future<Unit>::failure(Exception(api::ErrorCode::API_ERROR, fmt::format("Explorer returned truncated response with less then {} operations", self->_maxTransactionPerResponse)));
                            }
                            if (highestBlock.height != from) { // Explorer garanties us that we always get at least one full block
                                lastFullBlockHeight = highestBlock.height - 1;
                            }
                        }
                        uint32_t limit = std::min(lastFullBlockHeight, to); // want only transactions from untruncated blocks
                        for (auto &tr : bulk->first) {
                            if (tr.block.getValue().height <= limit) {
                                partialDB->addTransaction(tr);
                            }
                        }
                        std::vector<Future<Unit>> tasksToContinueWith;
                        if (isGap) {
                            auto newBatch = std::make_shared<Batch>();
                            for (auto &tr : bulk->first) {
                                if (tr.block.getValue().height > limit) continue;
                                for (auto& out : tr.outputs) {
                                    if (!out.address.hasValue()) continue;
                                    keychain->markAsUsed(out.address.getValue());
                                }
                            }
                            newBatch->addresses = keychain->getAddresses(batch->lastAddressIndex + 1, self->_batchSize);
                            newBatch->lastAddressIndex = batch->lastAddressIndex + self->_batchSize;
                            tasksToContinueWith.push_back(self->createBatchSyncTask(state, partialDB, newBatch, keychain, lowestBlock.height, to, lowestBlock.hash, true));
                        }
                        tasksToContinueWith.push_back(self->createBatchSyncTask(state, partialDB, batch, keychain, lastFullBlockHeight + 1, to, highestBlock.hash, false));
                        self->finilizeBatch(state, partialDB, from, to);
                        return executeAll(self->_executionContext, tasksToContinueWith).map<Unit>(self->_executionContext, [](const std::vector<Unit>& vu) { return unit; });
                    });
                }
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<Explorer> _explorer;
                std::shared_ptr<Keychain> _receiveKeychain;
                std::shared_ptr<Keychain> _changeKeychain;
                std::shared_ptr<BlockchainDatabase<NetworkType>> _blocksDB;
                const uint32_t _gapSize;
                const uint32_t _batchSize;
                const uint32_t _maxTransactionPerResponse;
            };
        }
    }
}