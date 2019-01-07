#pragma once

#include <mutex>
#include <memory>
#include <vector>

#include <wallet/NetworkTypes.hpp>
#include <async/FutureUtils.hpp>

namespace ledger {
    namespace core {
        template<typename T>
        class PartialBlockStorage;

        class Keychain;

        template<typename T>
        class BlockchainDatabase;

        template<typename Network>
        class ExplorerV2;

        namespace api {
            class ExecutionContext;
        };
        namespace common {
            

            namespace {

                struct Batch {
                    std::vector<std::string> addresses;
                    uint32_t lastAddressIndex;
                };

                class BlockSyncState {
                public:
                    BlockSyncState();
                    bool finishBatch();

                    void addBatch();
                private:
                    bool isCompleted();
                private:
                    std::mutex _lock;
                    uint32_t _batchesToSync;
                    bool _wasFinished;
                };

                class BlocksSyncState {
                public:
                    BlocksSyncState(uint32_t from, uint32_t to);

                    bool finishBatch(uint32_t blockHeight);
                    void addBatch(uint32_t from, uint32_t to);
                private:
                    std::vector<BlockSyncState> _blocks;
                    const uint32_t fromHeight;
                };
            }

            template<typename NetworkType>
            class BlocksSynchronizer : public std::enable_shared_from_this<BlocksSynchronizer<NetworkType>>{
            public:
                typedef typename NetworkType::Block Block;
                typedef typename NetworkType::Transaction Transaction;
                typedef typename NetworkType::FilledBlock FilledBlock;
                typedef BlockchainDatabase<FilledBlock> BlocksDatabase;

                BlocksSynchronizer(
                    const std::shared_ptr<api::ExecutionContext> executionContext,
                    const std::shared_ptr<ExplorerV2<NetworkType>>& explorer,
                    const std::shared_ptr<Keychain>& receiveKeychain,
                    const std::shared_ptr<Keychain>& changeKeychain,
                    uint32_t gapSize,
                    uint32_t batchSize,
                    uint32_t maxTransactionPerResponse);
                Future<Unit> synchronize(
                    const std::shared_ptr<BlocksDatabase>& blocksDB,
                    const std::string& blockHashToStart,
                    uint32_t firstBlockHeightToInclude,
                    uint32_t lastBlockHeightToInclude);
               private:

                Future<Unit> createBatchSyncTask(
                    const std::shared_ptr<BlocksDatabase>& blocksDB,
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<Transaction>>& db,
                    const std::shared_ptr<Batch>& batch,
                    const std::shared_ptr<Keychain>& keychain,
                    uint32_t from,
                    uint32_t to,
                    const std::string& firstBlockHash,
                    bool isGap);

                void bootstrapBatchesTasks(
                    const std::shared_ptr<BlocksDatabase>& blocksDB,
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<Transaction>>& db,
                    const std::shared_ptr<Keychain>& keychain,
                    std::vector<Future<Unit>>& tasks,
                    std::string firstBlockHash,
                    uint32_t from,
                    uint32_t to);

                void finilizeBatch(
                    const std::shared_ptr<BlocksDatabase>& blocksDB,
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<Transaction>>& partialDB,
                    int from,
                    int to);

                Future<Unit> synchronizeBatch(
                    const std::shared_ptr<BlocksDatabase>& blocksDB,
                    const std::shared_ptr<BlocksSyncState>& state,
                    const std::shared_ptr<PartialBlockStorage<Transaction>>& partialDB,
                    const std::shared_ptr<Batch>& batch,
                    const std::shared_ptr<Keychain>& keychain,
                    uint32_t from,
                    uint32_t to,
                    const std::string& hashToStartRequestFrom,
                    bool isGap);
            private:
                std::shared_ptr<api::ExecutionContext> _executionContext;
                std::shared_ptr<ExplorerV2<NetworkType>> _explorer;
                std::shared_ptr<Keychain> _receiveKeychain;
                std::shared_ptr<Keychain> _changeKeychain;
                const uint32_t _gapSize;
                const uint32_t _batchSize;
                const uint32_t _maxTransactionPerResponse;
            };
        }
    }
}

extern template class ledger::core::common::BlocksSynchronizer<ledger::core::BitcoinLikeNetwork>;
extern template class std::shared_ptr<ledger::core::common::BlocksSynchronizer<ledger::core::BitcoinLikeNetwork>>;
