/*
 *
 * AbstractBlockchainExplorerAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 29/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#ifndef LEDGER_CORE_ABSTRACTBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_ABSTRACTBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H

#include <algorithm>
#include <memory>
#include <mutex>

#include <api/Configuration.hpp>
#include <api/ConfigurationDefaults.hpp>
#include <async/Future.hpp>
#include <async/wait.h>
#include <collections/DynamicObject.hpp>
#include <debug/Benchmarker.h>
#include <events/ProgressNotifier.h>
#include <utils/Unit.hpp>
#include <utils/DateUtils.hpp>
#include <utils/DurationUtils.h>
#include <utils/Try.hpp>
#include <preferences/Preferences.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/common/database/AccountDatabaseHelper.h>

namespace ledger {
    namespace core {

        struct BlockchainExplorerAccountSynchronizationBatchSavedState {
            std::string blockHash;
            uint32_t blockHeight;

            BlockchainExplorerAccountSynchronizationBatchSavedState() {
                blockHeight = 0;
            }

            template<class Archive>
            void serialize(Archive & archive) {
                archive(blockHash, blockHeight);
            };
        };

        struct BlockchainExplorerAccountSynchronizationSavedState {
            uint32_t halfBatchSize;
            std::vector<BlockchainExplorerAccountSynchronizationBatchSavedState> batches;
            std::map<std::string, std::string> pendingTxsHash;

            BlockchainExplorerAccountSynchronizationSavedState(): halfBatchSize(0) {
            }

            template<class Archive>
            void serialize(Archive & archive)
            {
                archive(halfBatchSize, batches, pendingTxsHash); // serialize things by passing them to the archive
            }
        };

        template<typename Account, typename AddressType, typename Keychain, typename Explorer>
        class AbstractBlockchainExplorerAccountSynchronizer {
        public:
            using Transaction = typename Explorer::Transaction;

            std::shared_ptr<ProgressNotifier<Unit>> synchronizeAccount(const std::shared_ptr<Account>& account) {
                std::lock_guard<std::mutex> lock(_lock);
                if (!_currentAccount) {
                    _currentAccount = account;
                    _notifier = std::make_shared<ProgressNotifier<Unit>>();
                    auto self = getSharedFromThis();
                    performSynchronization(account).onComplete(getSynchronizerContext(), [self] (const Try<Unit> &result) {
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
            };

            struct SynchronizationBuddy {
                std::shared_ptr<Preferences> preferences;
                std::shared_ptr<spdlog::logger> logger;
                std::chrono::system_clock::time_point startDate;
                std::shared_ptr<AbstractWallet> wallet;
                std::shared_ptr<DynamicObject> configuration;
                uint32_t halfBatchSize;
                std::shared_ptr<Keychain> keychain;
                Option<BlockchainExplorerAccountSynchronizationSavedState> savedState;
                Option<void *> token;
                std::shared_ptr<Account> account;
                std::map<std::string, std::string> transactionsToDrop;

                virtual ~SynchronizationBuddy() {

                };
            };

        protected:

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
            };

            virtual std::shared_ptr<SynchronizationBuddy> makeSynchronizationBuddy() {
                return std::make_shared<SynchronizationBuddy>();
            }

            Future<Unit> performSynchronization(const std::shared_ptr<Account> &account) {
                auto buddy = makeSynchronizationBuddy();
                buddy->account = account;
                buddy->preferences = std::static_pointer_cast<AbstractAccount>(account)->getInternalPreferences()
                        ->getSubPreferences(
                                "AbstractBlockchainExplorerAccountSynchronizer");
                buddy->logger = account->logger();
                buddy->startDate = DateUtils::now();
                buddy->wallet = account->getWallet();
                buddy->configuration = std::static_pointer_cast<AbstractAccount>(account)->getWallet()->getConfig();
                buddy->halfBatchSize = (uint32_t) buddy->configuration
                        ->getInt(api::Configuration::SYNCHRONIZATION_HALF_BATCH_SIZE)
                        .value_or(api::ConfigurationDefaults::KEYCHAIN_DEFAULT_OBSERVABLE_RANGE);
                buddy->keychain = account->getKeychain();
                buddy->savedState = buddy->preferences
                        ->template getObject<BlockchainExplorerAccountSynchronizationSavedState>("state");
                buddy->logger
                        ->info("Starting synchronization for account#{} ({}) of wallet {} at {}",
                               account->getIndex(),
                               account->getKeychain()->getRestoreKey(),
                               account->getWallet()->getName(), DateUtils::toJSON(buddy->startDate));

                //Check if reorganization happened
                soci::session sql(buddy->wallet->getDatabase()->getPool());
                if (buddy->savedState.nonEmpty()) {

                    //Get deepest block saved in batches to be part of reorg
                    auto sortedBatches = buddy->savedState.getValue().batches;
                    std::sort(sortedBatches.begin(), sortedBatches.end(), [](const BlockchainExplorerAccountSynchronizationBatchSavedState &lhs,
                                                                             const BlockchainExplorerAccountSynchronizationBatchSavedState &rhs) -> bool {
                        return lhs.blockHeight < rhs.blockHeight;
                    });

                    auto currencyName = buddy->wallet->getCurrency().name;
                    size_t index = 0;
                    //Reorg can't happen until genesis block, safely initialize with 0
                    uint64_t deepestFailedBlockHeight = 0;
                    while (index < sortedBatches.size() && !BlockDatabaseHelper::blockExists(sql, sortedBatches[index].blockHash, currencyName)) {
                        deepestFailedBlockHeight = sortedBatches[index].blockHeight;
                        index ++;
                    }

                    //Case of reorg, update savedState's batches
                    if (deepestFailedBlockHeight > 0) {
                        //Get last block (in DB) which contains current account's operations
                        auto previousBlock = AccountDatabaseHelper::getLastBlockWithOperations(sql, buddy->account->getAccountUid());
                        for (auto& batch : buddy->savedState.getValue().batches) {
                            if (batch.blockHeight >= deepestFailedBlockHeight) {
                                batch.blockHeight = previousBlock.nonEmpty() ? (uint32_t)previousBlock.getValue().height : 0;
                                batch.blockHash = previousBlock.nonEmpty() ? previousBlock.getValue().blockHash : "";
                            }
                        }
                    }
                }

                initializeSavedState(buddy->savedState, buddy->halfBatchSize);

                updateTransactionsToDrop(sql, buddy, account->getAccountUid());

                updateCurrentBlock(buddy, account->getContext());

                auto self = getSharedFromThis();
                const auto deactivateToken =
                        buddy->configuration->getBoolean(api::Configuration::DEACTIVATE_SYNC_TOKEN).value_or(false);
                auto getSyncToken = deactivateToken ? Future<void *>::successful(nullptr) : _explorer->startSession();
                return getSyncToken.template map<Unit>(account->getContext(), [buddy, deactivateToken] (void * const t) -> Unit {
                    buddy->logger->info("Synchronization token obtained");
                    if (!deactivateToken && t) {
                        buddy->token = Option<void *>(t);
                    }
                    return unit;
                }).template flatMap<Unit>(account->getContext(), [buddy, self] (const Unit&) {
                    return self->synchronizeBatches(0, buddy);
                }).template flatMap<Unit>(account->getContext(), [self, buddy, deactivateToken] (const Unit&) {
                    if (deactivateToken) {
                        return Future<Unit>::successful(unit);
                    }
                    auto tryKillSession = Try<Future<Unit>>::from([=](){
                        return self->_explorer->killSession(buddy->token.getValue());
                    });
                    if (tryKillSession.isFailure()) {
                        buddy->logger->warn("Failed to delete synchronization token {} for account#{} of wallet {}",
                                            static_cast<char *>(buddy->token.getValue()), buddy->account->getIndex(),
                                            buddy->account->getWallet()->getName());
                        // We return a successful Unit because deleting the sync token should not be a "failure"
                        // Note: in a near future we'll try to get rid of sync token mechanism
                        return Future<Unit>::successful(unit);
                    }
                    return tryKillSession.getValue();
                }).template flatMap<Unit>(account->getContext(), [self, buddy] (auto) {
                    return self->synchronizeMempool(buddy);
                }).template map<Unit>(ImmediateExecutionContext::INSTANCE, [self, buddy] (const Unit&) {
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                    buddy->logger->info("End synchronization for account#{} of wallet {} in {}", buddy->account->getIndex(),
                                        buddy->account->getWallet()->getName(), DurationUtils::formatDuration(duration));

                    self->_currentAccount = nullptr;
                    return unit;
                }).recoverWith(ImmediateExecutionContext::INSTANCE, [self, buddy] (const Exception& ex) -> Future<Unit> {
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                    buddy->logger->error("Error during during synchronization for account#{} of wallet {} in {} ms", buddy->account->getIndex(),
                                         buddy->account->getWallet()->getName(), duration.count());
                    buddy->logger->error("Due to {}, {}", api::to_string(ex.getErrorCode()), ex.getMessage());
                    return self->recoverFromFailedSynchronization(buddy);
                });
            };

            // Synchronize batches.
            //
            // This function will synchronize all batches by iterating over batches and transactions
            // bulks. The input buddy can be used to customize the behavior of the synchronization.
            Future<Unit> synchronizeBatches(uint32_t currentBatchIndex,
                                            std::shared_ptr<SynchronizationBuddy> buddy) {
                buddy->logger->info("SYNC BATCHES");
                //For ETH and XRP like wallets, one account corresponds to one ETH address,
                //so ne need to discover other batches
                auto hasMultipleAddresses = buddy->wallet->getWalletType() == api::WalletType::BITCOIN;
                auto done = currentBatchIndex >= buddy->savedState.getValue().batches.size() - 1;
                if (currentBatchIndex >= buddy->savedState.getValue().batches.size()) {
                    buddy->savedState.getValue().batches.push_back(BlockchainExplorerAccountSynchronizationBatchSavedState());
                }

                auto self = getSharedFromThis();
                auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];

                auto benchmark = std::make_shared<Benchmarker>(fmt::format("Synchronize batch {}", currentBatchIndex), buddy->logger);
                benchmark->start();
                return synchronizeBatch(currentBatchIndex, buddy).template flatMap<Unit>(buddy->account->getContext(), [=] (const bool& hadTransactions) -> Future<Unit> {
                    benchmark->stop();

                    buddy->preferences->editor()->template putObject<BlockchainExplorerAccountSynchronizationSavedState>("state", buddy->savedState.getValue())->commit();

                    //Sync stops if there are no more batches in savedState and last batch has no transactions
                    //But we may want to force sync of accounts within KEYCHAIN_OBSERVABLE_RANGE
                    auto discoveredAddresses = currentBatchIndex * buddy->halfBatchSize;
                    auto lastDiscoverableAddress = buddy->configuration->getInt(api::Configuration::KEYCHAIN_OBSERVABLE_RANGE).value_or(buddy->halfBatchSize);
                    if (hasMultipleAddresses && (!done || (done && hadTransactions) || lastDiscoverableAddress > discoveredAddresses)) {
                        return self->synchronizeBatches(currentBatchIndex + 1, buddy);
                    }

                    return Future<Unit>::successful(unit);
                }).recoverWith(ImmediateExecutionContext::INSTANCE, [=] (const Exception &exception) -> Future<Unit> {
                    buddy->logger->info("Recovering from failing synchronization : {}", exception.getMessage());

                    //A block reorganization happened
                    if (exception.getErrorCode() == api::ErrorCode::BLOCK_NOT_FOUND &&
                        buddy->savedState.nonEmpty()) {
                        buddy->logger->info("Recovering from reorganization");

                        // Try to get a new sync token
                        const auto deactivateToken =
                                buddy->configuration->getBoolean(api::Configuration::DEACTIVATE_SYNC_TOKEN).value_or(false);
                        auto startSession = Future<void *>::async(ImmediateExecutionContext::INSTANCE, [=](){
                            if (deactivateToken) {
                                return Future<void *>::successful(nullptr);
                            }
                            return self->_explorer->startSession();
                        });

                        return startSession.template flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (void * const session) {
                            if (!deactivateToken && session) {
                                buddy->token = Option<void *>(session);
                            } else {
                                buddy->logger->warn(
                                        "Failed to get new synchronization token for account#{} of wallet {}",
                                        buddy->account->getIndex(),
                                        buddy->account->getWallet()->getName());
                                // WARNING: we have too many issues with that sync token because of blockchain explorer,
                                // when we fail on reorg we try without sync token
                                buddy->token = Option<void *>();
                            }

                            //Get its block/block height
                            auto &failedBatch = buddy->savedState.getValue().batches[currentBatchIndex];
                            auto const failedBlockHeight = failedBatch.blockHeight;
                            auto const failedBlockHash = failedBatch.blockHash;

                            if (failedBlockHeight > 0) {
                                //Delete data related to failedBlock (and all blocks above it)
                                buddy->logger->info("Deleting blocks above block height: {}", failedBlockHeight);

                                soci::session sql(buddy->wallet->getDatabase()->getPool());
                                {
                                    soci::transaction tr(sql);
                                    try {
                                        sql << "DELETE FROM blocks where height >= :failedBlockHeight", soci::use(
                                                failedBlockHeight);

                                        //Get last block not part from reorg
                                        auto lastBlock = BlockDatabaseHelper::getLastBlock(sql,
                                                                                           buddy->wallet->getCurrency().name);

                                        //Resync from the "beginning" if no last block in DB
                                        int64_t lastBlockHeight = 0;
                                        std::string lastBlockHash;
                                        if (lastBlock.nonEmpty()) {
                                            lastBlockHeight = lastBlock.getValue().height;
                                            lastBlockHash = lastBlock.getValue().blockHash;
                                        }

                                        //Update savedState's batches
                                        for (auto &batch : buddy->savedState.getValue().batches) {
                                            if (batch.blockHeight > lastBlockHeight) {
                                                batch.blockHeight = (uint32_t) lastBlockHeight;
                                                batch.blockHash = lastBlockHash;
                                            }
                                        }
                                        tr.commit();
                                    } catch(...) {
                                        tr.rollback();
                                    }
                                }

                                //Save new savedState
                                buddy->preferences->editor()->template putObject<BlockchainExplorerAccountSynchronizationSavedState>(
                                        "state", buddy->savedState.getValue())->commit();

                                //Synchronize same batch now with an existing block (of hash lastBlockHash)
                                //if failedBatch was not the deepest block part of that reorg, this recursive call
                                //will ensure to get (and delete from DB) to the deepest failed block (part of reorg)
                                buddy->logger->info("Relaunch synchronization after recovering from reorganization");

                                return self->synchronizeBatches(currentBatchIndex, buddy);
                            }
                            return Future<Unit>::successful(unit);
                        }).recover(ImmediateExecutionContext::INSTANCE, [buddy] (const Exception& ex) -> Unit {
                            buddy->logger->warn(
                                    "Failed to recover from reorganisation for account#{} of wallet {}",
                                    buddy->account->getIndex(),
                                    buddy->account->getWallet()->getName());
                            return unit;
                        });
                    }
                    return Future<Unit>::successful(unit);
                });
            };

            // Synchronize a transactions batch.
            //
            // The currentBatchIndex is the currently synchronized batch. buddy is the
            // synchronization object used to accumulate a state. hadTransactions is used to check
            // whether more data is needed. If a block doesn’t have any transaction, it means that
            // we must stop.
            Future<bool> synchronizeBatch(
                    uint32_t currentBatchIndex,
                    std::shared_ptr<SynchronizationBuddy> buddy,
                    bool hadTransactions = false) {
                buddy->logger->info("SYNC BATCH {}", currentBatchIndex);

                Option<std::string> blockHash;
                auto self = getSharedFromThis();
                auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];

                if (batchState.blockHeight > 0) {
                    blockHash = Option<std::string>(batchState.blockHash);
                }

                auto derivationBenchmark = std::make_shared<Benchmarker>("Batch derivation", buddy->logger);
                derivationBenchmark->start();

                auto batch = vector::map<std::string, std::shared_ptr<AddressType>>(
                        buddy->keychain->getAllObservableAddresses((uint32_t) (currentBatchIndex * buddy->halfBatchSize),
                                                                   (uint32_t) ((currentBatchIndex + 1) * buddy->halfBatchSize - 1)),
                        [] (const std::shared_ptr<AddressType>& addr) -> std::string {
                            return addr->toString();
                        }
                );

                derivationBenchmark->stop();

                auto benchmark = std::make_shared<Benchmarker>("Get batch", buddy->logger);
                benchmark->start();
                return _explorer
                    ->getTransactions(batch, blockHash, buddy->token)
                    .template flatMap<bool>(buddy->account->getContext(), [self, currentBatchIndex, buddy, hadTransactions, benchmark] (const std::shared_ptr<typename Explorer::TransactionsBulk>& bulk) -> Future<bool> {
                        benchmark->stop();

                        auto insertionBenchmark = std::make_shared<Benchmarker>("Transaction computation", buddy->logger);
                        insertionBenchmark->start();

                        auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];
                        soci::session sql(buddy->wallet->getDatabase()->getPool());
                        buddy->logger->info("Got {} txs for account {}", bulk->transactions.size(), buddy->account->getAccountUid());
                        auto count = 0;
                        for (const auto& tx : bulk->transactions) {
                            soci::transaction tr(sql);
                            // A lot of things could happen here, better to wrap it
                            auto tryPutTx = Try<int>::from([&buddy, &tx, &sql, &self] () {
                                auto flag = self->putTransaction(sql, tx, buddy);
                                //Update first pendingTxHash in savedState
                                auto it = buddy->transactionsToDrop.find(tx.hash);
                                if (it != buddy->transactionsToDrop.end()) {
                                    //If block non empty, tx is no longer pending
                                    if (tx.block.nonEmpty()) {
                                        buddy->savedState.getValue().pendingTxsHash.erase(it->first);
                                    } else { //Otherwise tx is in mempool but pending
                                        buddy->savedState.getValue().pendingTxsHash.insert(std::pair<std::string, std::string>(it->first, it->second));
                                    }
                                }
                                //Remove from tx to drop
                                buddy->transactionsToDrop.erase(tx.hash);
                                return flag;
                            });

                            if (tryPutTx.isFailure()) {
                                tr.rollback();
                                auto blockHash = tx.block.hasValue() ? tx.block.getValue().hash : "None";
                                buddy->logger->error("Failed to put transaction {}, on block {}, for account {}, reason: {}, rollback ...", tx.hash, blockHash, buddy->account->getAccountUid(), tryPutTx.getFailure().getMessage());
                            } else {
                                count++;
                                tr.commit();
                            }
                        }
                        buddy->logger->info("Succeeded to insert {} txs on {} for account {}", count, bulk->transactions.size(), buddy->account->getAccountUid());
                        buddy->account->emitEventsNow();

                        // Get the last block
                        if (bulk->transactions.size() > 0) {
                            auto &lastBlock = bulk->transactions.back().block;

                            if (lastBlock.nonEmpty()) {
                                batchState.blockHeight = (uint32_t) lastBlock.getValue().height;
                                batchState.blockHash = lastBlock.getValue().hash;
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
            };

            virtual Future<Unit> synchronizeMempool(const std::shared_ptr<SynchronizationBuddy>& buddy) {
                //Delete dropped txs from DB
                soci::session sql(buddy->wallet->getDatabase()->getPool());
                for (auto& tx : buddy->transactionsToDrop) {
                    //Check if tx is pending
                    auto it = buddy->savedState.getValue().pendingTxsHash.find(tx.first);
                    if (it == buddy->savedState.getValue().pendingTxsHash.end()) {
                        soci::transaction tr(sql);
                        buddy->logger->info("Drop transaction {}", tx.first);
                        buddy->logger->info("Deleting operation from DB {}", tx.second);
                        try {
                            sql << "DELETE FROM operations WHERE uid = :uid", soci::use(tx.second);
                            tr.commit();
                        } catch(std::exception& ex) {
                            buddy->logger->info("Failed to delete operation from DB {} reason: {}, rollback ...", tx.second, ex.what());
                            tr.rollback();
                        }
                    }
                }
                return Future<Unit>::successful(unit);
            }

            virtual Future<Unit> recoverFromFailedSynchronization(const std::shared_ptr<SynchronizationBuddy>& buddy) {
                return Future<Unit>::successful(unit);
            }

            virtual int putTransaction(soci::session& sql, const Transaction& transaction, const std::shared_ptr<SynchronizationBuddy>& buddy) = 0;

            virtual void updateCurrentBlock(std::shared_ptr<SynchronizationBuddy> &buddy,
                                            const std::shared_ptr<api::ExecutionContext> &context) = 0;
            virtual void updateTransactionsToDrop(soci::session &sql,
                                                  std::shared_ptr<SynchronizationBuddy> &buddy,
                                                  const std::string &accountUid) = 0;



            std::shared_ptr<Explorer> _explorer;
            std::shared_ptr<ProgressNotifier<Unit>> _notifier;
            std::mutex _lock;
            std::shared_ptr<Account> _currentAccount;

        private:

            virtual std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer<Account, AddressType, Keychain, Explorer>> getSharedFromThis() = 0;
            virtual std::shared_ptr<api::ExecutionContext> getSynchronizerContext() = 0;

            std::shared_ptr<Preferences> _internalPreferences;
        };
    }
}


#endif //LEDGER_CORE_ABSTRACTBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
