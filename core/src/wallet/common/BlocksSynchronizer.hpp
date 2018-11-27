#pragma once

#include <mutex>
#include <memory>
#include <vector>

#include <api/ExecutionContext.hpp>
#include <async/FutureUtils.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/Explorer.hpp>
#include <wallet/Keychain.hpp>

namespace ledger {
    namespace core {
        namespace common {
            namespace {

                struct Batch {
                    std::vector<std::string> addresses;
                };

                class BlockSyncState {
                public:
                    BlockSyncState(uint32_t batchesToSync)
                        : _batchesToSync(batchesToSync)
                        , _gappedChangeExplored(false)
                        , _gappedReceiveExplored(false){
                    }
                    bool finishBatch() {
                        std::lock_guard<std::mutex> lock(_lock);
                        _batchesToSync--;
                        return isCompleted();
                    }

                    bool finishGapped(keychain::KeyPurpose purpose) {
                        std::lock_guard<std::mutex> lock(_lock);
                        if (purpose == keychain::CHANGE) {
                            _gappedChangeExplored = true;
                        }
                        else {
                            _gappedReceiveExplored = true;
                        }
                        return isCompleted();
                    }
                    
                private:
                    bool isCompleted() {
                        return (_batchesToSync == 0) && _gappedChangeExplored && _gappedReceiveExplored;
                    }
                    std::mutex _lock;
                    uint32_t _batchesToSync;
                    bool _gappedChangeExplored;
                    bool _gappedReceiveExplored;
                };

                class BlocksSyncState {
                public:
                    BlocksSyncState(uint32_t from, uint32_t to, uint32_t numberOfTasks)
                        : fromHeight(from)
                        , toHeight(to)
                        , _numberOfTasks(numberOfTasks) {
                        for (int i = 0; i <= to - from; ++i) {
                            _blocks.push_back(std::make_shared<BlockSyncState>(_numberOfTasks));
                        }
                    }

                    bool finishBatch(uint32_t blockHeight) {
                        return _blocks[blockHeight - fromHeight]->finishBatch();
                    }

                    bool finishGapped(uint32_t blockHeight, keychain::KeyPurpose purpose) {
                        return _blocks[blockHeight - toHeight]->finishGapped(purpose);
                    }
                public:
                    const uint32_t fromHeight;
                    const uint32_t toHeight;
                private:
                    const uint32_t _numberOfTasks;
                    std::vector<std::shared_ptr<BlockSyncState>> _blocks;
                };
            }

            template<typename NetworkType>
            class BlocksSynchronizer {
            public:
                typedef ExplorerV2<NetworkType> Explorer;
                typedef typename Explorer::TransactionBulk TransactionBulk;
                typedef typename NetworkType::Block Block;
                typedef typename NetworkType::Transaction Transaction;
                typedef typename NetworkType::FilledBlock FilledBlock;

                BlocksSynchronizer(
                    const std::shared_ptr<api::ExecutionContext> executionContext,
                    const std::shared_ptr<Explorer>& explorer,
                    const std::shared_ptr<Keychain<NetworkType>>& receiveKeychain,
                    const std::shared_ptr<Keychain<NetworkType>>& changeKeychain,
                    const std::shared_ptr<BlockchainDatabase<NetworkType>>& blocksDB,
                    uint32_t gapSize,
                    uint32_t batchSize)
                : _executionContext(executionContext)
                , _explorer(explorer)
                , _receivekeychain(receiveKeychain)
                , _changeKeychain(changeKeychain)
                , _blockDB(blocksDB)
                , _gapSize(gapSize)
                , _batchSize(batchSize) {
                }

                Future<Unit> synchronize(
                    const std::shared_ptr<Block>& firstBlock,
                    const std::shared_ptr<Block>& lastBlock) {
                    std::vector<Future<Unit>> tasks; 
                    uint32_t numberOfReceiveAddresses = _receiveKeychain->getNumberOfUsedAddresses();
                    uint32_t numberOfChangeAddresses = _changeKeychain->getNumberOfUsedAddresses(); 
                    uint32_t numberOfBatches = numberOfReceiveAddresses / _batchSize + (((numberOfReceiveAddresses % _batchSize) == 0) ? 0 : 1);
                    numberOfBatches += numberOfChangeAddresses / _batchSize + (((numberOfChangeAddresses % _batchSize) == 0) ? 0 : 1);
                    auto partialBlockDB = std::make_shared<InMemoryPartialBlocksDB<NetworkType>>();
                    auto state = std::make_shared<BlocksSyncState>(firstBlock->height, lastBlock->height, numberOfBatches);
                    bootstrapBatchesTasks(state, partialBlockDB, numberOfReceiveAddresses, _receiveKeychain, tasks);
                    bootstrapBatchesTasks(state, partialBlockDB, numberOfChangeAddresses, _changeKeychain, tasks);
                    return executeAll(_executionContext, tasks).map<Unit>(_executionContext, [](const std::vector<Unit>& vu) { return unit; });
                }
            private:

                void bootstrapBatchesTasks(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& db,
                    uint32_t numberOfAddresses,
                    const std::shared_ptr<Keychain<NetworkType>>& keychain,
                    std::vector<Future<Unit>>& tasks) {
                    for (int i = 0; i < numberOfAddresses; i += _batchSize) {
                        auto batch = std::make_shared<Batch>();
                        batch->addresses = keychain->getAddresses(i, std::min(_batchSize, numberOfAddresses - i));
                        tasks.push_back(synchronizeBatch(state, db, batch, keychain));
                    }
                    // add gapped addresses
                    auto batch = std::make_shared<Batch>();
                    batch->addresses = keychain->getAddresses(numberOfAddresses, _gapSize);
                    tasks.push_back(synchronizeGap(state, db, batch, keychain));
                }

                Future<Unit> synchronizeGap(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& partialDB,
                    const std::shared_ptr<Batch>& batch,
                    const std::shared_ptr<Keychain<NetworkType>>& keychain ) {
                    return Future<Unit>::failure(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me");
                }

                Future<Unit> finilizeBatch(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& partialDB,
                    int from,
                    int to) {
                    std::vector<FilledBlock> blocks;
                    for (int i = from; i <= to; ++i) {
                        if (state->finishBatch(i)) {
                            FilledBlock block;
                            block.first = Block();
                            block.first.height = i;
                            auto trans = partialDB->getTransactions(i);
                            if (trans.size() != 0) {
                                block.hash = trans[0].block.getValue().hash;
                                block.second = trans;
                            }
                            else {
                                block.hash = "DIDN'T ASK EXPLORER ABOUT";
                            }
                            blocks.push_back(block);
                            // try to not consume to much memory
                            partialDB->removeBlock(i);
                        }
                    }
                    return _blocksDB->addBlocks(blocks);
                }

                Future<Unit> synchronizeBatch(
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<NetworkType>>& partialDB,
                    const std::shared_ptr<Batch>& batch,
                    const std::shared_ptr<Keychain<NetworkType>>& keychain,
                    uint32_t from,
                    uint32_t to,
                    std::string hashToStartRequestFrom) {
                    if (batch->addresses.size() == 0) { // special case for account-bases blockchains (ex ETH don't have "change" addresses)
                        return finilizeBatch(state, partialDB, from, to);
                    }
                    auto self = shared_from_this();
                    return
                        _explorer->getTransactions(batch->addresses, hashToStartRequestFrom)
                        .map<Unit>(_executionContext, [self, batch, partialDB, state](const std::shared_ptr<TransactionBulk>& bulk) {
                        uint32_t maxHeight = 1U << 31;
                        std::string nextHashToAsk;
                        if (bulk->second) { //response truncated
                            if (bulk->first.size() == 0) {
                                return Future<Unit>::failure(api::ErrorCode::API_ERROR, "Explorer return empty list of transactions and truncated indicator");
                            }
                            Block highestBlock = std::max<Transaction>(
                                bulk->first,
                                [](const Transaction& l, const Transaction& r) {return l.block.getValue().height < r.block.getValue().height; }).block.getValue();
                            maxHeight = highestBlock.height - 1;
                            nextHashToAsk = highestBlock.hash;
                            if (maxHeight < from->hash) { // security checks to avoid infinity loop
                                if (bulk->first.size() == 200) {
                                    return Future<Unit>::failure(api::ErrorCode::API_ERROR, "Can't handle more then 200 transaction per batch in a block.");
                                }
                                return Future<Unit>::failure(api::ErrorCode::API_ERROR, "Explorer return transaction with block height less then requested");
                            }
                        }
                        uint32_t limit = std::min(maxHeight, to); // want only confirmed transactions from untruncated blocks
                        for (auto &tr : bulk->first) {
                            if (tr.block.getValue().height <= limit)
                                partialDB->addTransaction(tr);
                        }
                        if (limit == to) {
                            return self->finilizeBatch(state, partialDB, from->height, limit);
                        }
                        if (nextHashToAsk.empty()) {
                            return Future<Unit>::failure(api::ErrorCode::API_ERROR, "Explorer return transaction without block hash");
                        }
                        return
                            self->finilizeBatch(state, partialDB, nextHashToAsk, from, limit)
                            .flartMap<Unit>(self->_executionContext, [self, state, partialDB, batch, ]);
                    });
                }
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<Explorer> _explorer;
                std::shared_ptr<Keychain<NetworkType>> _receiveKeychain;
                std::shared_ptr<Keychain<NetworkType>> _changeKeychain;
                std::shared_ptr<BlockchainDatabase<NetworkType>> _blocksDB;
                const uint32_t _gapSize;
                const uint32_t _batchSize;
            };
        }
    }
}