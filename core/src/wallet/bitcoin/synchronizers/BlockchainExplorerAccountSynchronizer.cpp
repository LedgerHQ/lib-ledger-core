/*
 *
 * BlockchainExplorerAccountSynchronizer
 * ledger-core
 *
 * Created by Pierre Pollastri on 26/05/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include "BlockchainExplorerAccountSynchronizer.h"
#include <mutex>
#include <async/wait.h>
#include <utils/DateUtils.hpp>
#include <cereal/types/vector.hpp>
#include <algorithm>
#include <async/algorithm.h>
#include <debug/Benchmarker.h>
#include <utils/DurationUtils.h>

namespace ledger {
    namespace core {

        BlockchainExplorerAccountSynchronizer::BlockchainExplorerAccountSynchronizer(
                const std::shared_ptr<WalletPool> &pool,
                const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer) :
                DedicatedContext(pool->getDispatcher()
                                     ->getThreadPoolExecutionContext("synchronizers")) {
            _explorer = explorer;
        }

        std::shared_ptr<ProgressNotifier<Unit>>
        BlockchainExplorerAccountSynchronizer::synchronize(const std::shared_ptr<BitcoinLikeAccount> &account) {
            std::lock_guard<std::mutex> lock(_lock);
            if (!_currentAccount) {
                _currentAccount = account;
                _notifier = std::make_shared<ProgressNotifier<Unit>>();
                auto self = shared_from_this();
                performSynchronization(account).onComplete(getContext(), [self] (const Try<Unit> &result) {
                    std::lock_guard<std::mutex> l(self->_lock);
                    if (result.isFailure()) {
                        self->_notifier->failure(result.getFailure());
                    } else {
                        self->_notifier->success(unit);
                    }
                    self->_notifier = nullptr;
                    self->_currentAccount = nullptr;
                });

            } else if (account != _currentAccount) {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "This synchronizer is already in use");
            }
            return _notifier;
        }

        bool BlockchainExplorerAccountSynchronizer::isSynchronizing() const {
            return _notifier != nullptr;
        }

        void BlockchainExplorerAccountSynchronizer::reset(const std::shared_ptr<BitcoinLikeAccount> &account,
                                                          const std::chrono::system_clock::time_point &toDate) {

        }

        static void initializeSavedState(Option<BlockchainExplorerAccountSynchronizationSavedState> &savedState,
                                                int32_t halfBatchSize) {
            if (savedState.hasValue() && savedState.getValue()
                                                   .halfBatchSize != halfBatchSize) {
                BlockchainExplorerAccountSynchronizationBatchSavedState block;
                block.blockHeight = 1U << 31U;

                for (auto &state : savedState.getValue()
                                             .batches) {
                    if (state.blockHeight < block.blockHeight) {
                        block = state;
                    }
                }
                auto newBatchCount = (savedState.getValue()
                                                .batches.size() * savedState.getValue()
                                                                        .halfBatchSize) / halfBatchSize;
                if ((savedState.getValue()
                               .batches.size() * savedState.getValue()
                                                       .halfBatchSize) / halfBatchSize != 0)
                    newBatchCount += 1;
                savedState.getValue()
                          .batches
                          .clear();
                savedState.getValue()
                          .halfBatchSize = (uint32_t) halfBatchSize;
                for (auto i = 0; i <= newBatchCount; i++) {
                    BlockchainExplorerAccountSynchronizationBatchSavedState s;
                    s.blockHash = block.blockHash;
                    s.blockHeight = block.blockHeight;
                    savedState.getValue().batches.push_back(s);
                }
            } else if (savedState.isEmpty()) {
                savedState = Option<BlockchainExplorerAccountSynchronizationSavedState>(
                        BlockchainExplorerAccountSynchronizationSavedState());
                savedState.getValue()
                          .halfBatchSize = (uint32_t) halfBatchSize;
            }
        }

        Future<Unit> BlockchainExplorerAccountSynchronizer::performSynchronization(
                const std::shared_ptr<BitcoinLikeAccount> &account) {
            auto buddy = std::make_shared<SynchronizationBuddy>();

            buddy->account = account;
            buddy->preferences = std::static_pointer_cast<AbstractAccount>(account)->getInternalPreferences()
                                                                                   ->getSubPreferences(
                                                                                           "BlockchainExplorerAccountSynchronizer");
            buddy->logger = account->logger();
            buddy->startDate = DateUtils::now();
            buddy->wallet = account->getWallet();
            buddy->configuration = std::static_pointer_cast<AbstractAccount>(account)->getWallet()
                                                                                     ->getConfiguration();
            buddy->halfBatchSize = (uint32_t) buddy->configuration
                                                   ->getInt(api::Configuration::SYNCHRONIZATION_HALF_BATCH_SIZE)
                                                   .value_or(10);
            buddy->keychain = account->getKeychain();
            buddy->savedState = buddy->preferences
                                     ->getObject<BlockchainExplorerAccountSynchronizationSavedState>("state");

            buddy->logger
                 ->info("Starting synchronization for account#{} ({}) of wallet {} at {}",
                        account->getIndex(),
                        account->getKeychain()
                               ->getRestoreKey(),
                        account->getWallet()
                               ->getName(), DateUtils::toJSON(buddy->startDate)
                 );

            initializeSavedState(buddy->savedState, buddy->halfBatchSize);
            auto self = shared_from_this();
            return _explorer->startSession().map<Unit>(account->getContext(), [buddy] (void * const& t) -> Unit {
                buddy->token = Option<void *>(t);
                return unit;
            }).flatMap<Unit>(account->getContext(), [buddy, self] (const Unit&) {
                return self->synchronizeBatches(0, buddy);
            }).flatMap<Unit>(account->getContext(), [self, buddy] (const Unit&) {
                return self->_explorer->killSession(buddy->token.getValue());
            }).map<Unit>(ImmediateExecutionContext::INSTANCE, [self, buddy] (const Unit&) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                buddy->logger->info("End synchronization for account#{} of wallet {} in {}", buddy->account->getIndex(),
                             buddy->account->getWallet()->getName(), DurationUtils::formatDuration(duration));
                self->_currentAccount = nullptr;
                return unit;
            }).recover(ImmediateExecutionContext::INSTANCE, [buddy] (const Exception& ex) -> Unit {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                buddy->logger->error("Error during during synchronization for account#{} of wallet {} in {} ms", buddy->account->getIndex(),
                                    buddy->account->getWallet()->getName(), duration.count());
                throw ex;
            });
        }

        Future<Unit> BlockchainExplorerAccountSynchronizer::synchronizeBatches(uint32_t currentBatchIndex,
                                                                               std::shared_ptr<BlockchainExplorerAccountSynchronizer::SynchronizationBuddy> buddy) {

            auto done = (currentBatchIndex >= buddy->savedState.getValue().batches.size() - 1);
            if (currentBatchIndex >= buddy->savedState.getValue().batches.size()) {
                buddy->savedState.getValue().batches.push_back(BlockchainExplorerAccountSynchronizationBatchSavedState());
            }
            auto self = shared_from_this();
            auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];
            auto benchmark = std::make_shared<Benchmarker>(fmt::format("Synchronize batch {}", currentBatchIndex), buddy->logger);
            benchmark->start();
            return synchronizeBatch(currentBatchIndex, buddy).flatMap<Unit>(buddy->account->getContext(), [=] (const bool& hadTransactions) -> Future<Unit> {
                benchmark->stop();
                buddy->preferences->editor()->putObject<BlockchainExplorerAccountSynchronizationSavedState>("state", buddy->savedState.getValue())->commit();
                if (!done || (done && hadTransactions)) {
                    return self->synchronizeBatches(currentBatchIndex + 1, buddy);
                }
                return Future<Unit>::successful(unit);
            });
        }

        Future<bool> BlockchainExplorerAccountSynchronizer::synchronizeBatch(uint32_t currentBatchIndex,
                                                                             std::shared_ptr<BlockchainExplorerAccountSynchronizer::SynchronizationBuddy> buddy,
                                                                             bool hadTransactions) {
            Option<std::string> blockHash;
            auto self = shared_from_this();
            auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];
            if (batchState.blockHeight > 0)
                blockHash = Option<std::string>(batchState.blockHash);
            auto derivationBenchmark = std::make_shared<Benchmarker>("Batch derivation", buddy->logger);
            derivationBenchmark->start();
            auto batch = buddy->keychain->getAllObservableAddresses((uint32_t) (currentBatchIndex * buddy->halfBatchSize),
                                                                    (uint32_t) ((currentBatchIndex + 1) * buddy->halfBatchSize - 1));
            derivationBenchmark->stop();
            auto benchmark = std::make_shared<Benchmarker>("Get batch", buddy->logger);
            benchmark->start();
            return _explorer
                    ->getTransactions(batch, blockHash, buddy->token)
                    .flatMap<bool>(buddy->account->getContext(), [self, currentBatchIndex, buddy, hadTransactions, benchmark] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::TransactionsBulk>& bulk) -> Future<bool> {
                        benchmark->stop();
                        auto insertionBenchmark = std::make_shared<Benchmarker>("Transaction computation", buddy->logger);
                        insertionBenchmark->start();
                        auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];
                        soci::session sql(buddy->wallet->getDatabase()->getPool());
                        soci::transaction tr(sql);
                        for (const auto& tx : bulk->transactions) {
                            buddy->account->putTransaction(sql, tx);
                        }
                        tr.commit();
                        buddy->account->emitEventsNow();
                        // Get the last block
                        if (bulk->transactions.size() > 0) {
                            auto &lastBlock = bulk->transactions
                                                  .back()
                                                  .block;
                            if (lastBlock.nonEmpty()) {
                                batchState.blockHeight = (uint32_t) lastBlock.getValue()
                                                                             .height;
                                batchState.blockHash = lastBlock.getValue()
                                                                .hash;
                            }
                        }
                        insertionBenchmark->stop();
                        auto hadTX = hadTransactions || bulk->transactions.size() > 0;
                        if (bulk->hasNext) {
                            return self->synchronizeBatch(currentBatchIndex, buddy, hadTX);
                        } else {
                            return Future<bool>::successful(hadTX);
                        }
                    });
        }
    }
}