/*
 *
 * TezosLikeAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include <api/ConfigurationDefaults.hpp>
#include <async/Future.hpp>
#include <collections/DynamicObject.hpp>
#include <debug/Benchmarker.h>
#include <events/ProgressNotifier.h>
#include <utils/Unit.hpp>
#include <utils/DateUtils.hpp>
#include <utils/DurationUtils.h>
#include <utils/Try.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <common/AccountHelper.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/tezos/TezosLikeAccount.h>
#include <wallet/tezos/synchronizers/TezosLikeAccountSynchronizer.hpp>

#include <algorithm>

namespace ledger {
namespace core {

namespace {

void initializeSavedState(
    Option<tezos::AccountSynchronizationSavedState> &savedState,
    int32_t halfBatchSize)
{
    if (savedState.hasValue() && savedState.getValue().halfBatchSize != halfBatchSize) {
        tezos::AccountSynchronizationBatchSavedState block;
        block.blockHeight = 1U << 31U;

        for (auto &state : savedState.getValue().batches) {
            if (state.blockHeight < block.blockHeight) {
                block = state;
            }
        }
        auto newBatchCount =
            (savedState.getValue().batches.size() * savedState.getValue().halfBatchSize) /
            halfBatchSize;
        if (newBatchCount != 0) {
            ++newBatchCount;
        }
        savedState.getValue().batches.clear();
        savedState.getValue().halfBatchSize = static_cast<uint32_t>(halfBatchSize);
        for (auto i = 0; i <= newBatchCount; i++) {
            tezos::AccountSynchronizationBatchSavedState s;
            s.blockHash = block.blockHash;
            s.blockHeight = block.blockHeight;
            savedState.getValue().batches.push_back(s);
        }
    } else if (savedState.isEmpty()) {
        savedState = Option<tezos::AccountSynchronizationSavedState>(tezos::AccountSynchronizationSavedState());
        savedState.getValue().halfBatchSize = static_cast<uint32_t>(halfBatchSize);
    }
}

} // namespace

TezosLikeAccountSynchronizer::TezosLikeAccountSynchronizer(
    const std::shared_ptr<WalletPool>& pool,
    const std::shared_ptr<TezosLikeBlockchainExplorer>& explorer)
    : DedicatedContext(pool->getDispatcher()->getThreadPoolExecutionContext("synchronizers"))
    , _explorer(explorer)
{}

std::shared_ptr<ProgressNotifier<tezos::AccountSynchronizationContext>>
TezosLikeAccountSynchronizer::synchronizeAccount(const std::shared_ptr<TezosLikeAccount>& account)
{
    std::lock_guard<std::mutex> lock(_lock);
    if (!_currentAccount) {
        _currentAccount = account;
        _notifier = std::make_shared<ProgressNotifier<tezos::AccountSynchronizationContext>>();
        auto self = shared_from_this();
        performSynchronization(account).onComplete(getContext(), [self] (const auto& result) {
            std::lock_guard<std::mutex> l(self->_lock);
            if (result.isFailure()) {
                self->_notifier->failure(result.getFailure());
            } else {
                self->_notifier->success(result.getValue());
            }
            self->_notifier = nullptr;
            self->_currentAccount = nullptr;
        });

    } else if (account != _currentAccount) {
        throw make_exception(api::ErrorCode::RUNTIME_ERROR, "This synchronizer is already in use");
    }
    return _notifier;
}

Future<tezos::AccountSynchronizationContext>
TezosLikeAccountSynchronizer::performSynchronization(const std::shared_ptr<TezosLikeAccount>& account)
{
    auto buddy = std::make_shared<tezos::SynchronizationBuddy>();
    buddy->account = account;
    buddy->preferences = std::static_pointer_cast<AbstractAccount>(account)
        ->getInternalPreferences()
        ->getSubPreferences("TezosLikeAccountSynchronizer");
    auto loggerPurpose = fmt::format("synchronize_{}", account->getAccountUid());
    auto tracePrefix = fmt::format(
        "{}/{}/{}",
        account->getWallet()->getPool()->getName(),
        account->getWallet()->getName(),
        account->getIndex());
    buddy->synchronizationTag = tracePrefix;
    buddy->logger = logger::trace(loggerPurpose, tracePrefix, account->logger());
    buddy->startDate = DateUtils::now();
    buddy->wallet = account->getWallet();
    buddy->configuration = account->getWallet()->getConfig();
    buddy->halfBatchSize = static_cast<uint32_t>(buddy->configuration
            ->getInt(api::Configuration::SYNCHRONIZATION_HALF_BATCH_SIZE)
            .value_or(api::ConfigurationDefaults::KEYCHAIN_DEFAULT_OBSERVABLE_RANGE));
    buddy->keychain = account->getKeychain();
    buddy->savedState = buddy->preferences->getObject<tezos::AccountSynchronizationSavedState>("state");
    buddy->logger->info(
        "Starting synchronization for account#{} ({}) of wallet {} at {}",
        account->getIndex(),
        account->getKeychain()->getRestoreKey(),
        account->getWallet()->getName(), DateUtils::toJSON(buddy->startDate));

    auto fullSyncBenchmarker = std::make_shared<Benchmarker>(
        fmt::format("full_synchronization/{}", buddy->synchronizationTag),
        buddy->logger);

    fullSyncBenchmarker->start();
    //Check if reorganization happened
    soci::session sql(buddy->wallet->getDatabase()->getPool());
    if (buddy->savedState.nonEmpty()) {

        //Get deepest block saved in batches to be part of reorg
        auto sortedBatches = buddy->savedState.getValue().batches;
        std::sort(
            sortedBatches.begin(),
            sortedBatches.end(),
            [](const tezos::AccountSynchronizationBatchSavedState &lhs,
               const tezos::AccountSynchronizationBatchSavedState &rhs) {
                return lhs.blockHeight < rhs.blockHeight;
            });

        auto currencyName = buddy->wallet->getCurrency().name;
        size_t index = 0;
        //Reorg can't happen until genesis block, safely initialize with 0
        uint64_t deepestFailedBlockHeight = 0;
        while (index < sortedBatches.size() &&
            !BlockDatabaseHelper::blockExists(sql, sortedBatches[index].blockHash, currencyName)) {
            deepestFailedBlockHeight = sortedBatches[index].blockHeight;
            index++;
        }

        //Case of reorg, update savedState's batches
        if (deepestFailedBlockHeight > 0) {
            //Get last block (in DB) which contains current account's operations
            auto previousBlock = AccountDatabaseHelper::getLastBlockWithOperations(
                sql, buddy->account->getAccountUid());
            for (auto& batch : buddy->savedState.getValue().batches) {
                if (batch.blockHeight >= deepestFailedBlockHeight) {
                    batch.blockHeight = previousBlock.nonEmpty() ?
                        static_cast<uint32_t>(previousBlock.getValue().height) : 0;
                    batch.blockHash = previousBlock.nonEmpty() ?
                        previousBlock.getValue().blockHash : "";
                }
            }
        }
    }

    initializeSavedState(buddy->savedState, buddy->halfBatchSize);

    updateTransactionsToDrop(sql, buddy, account->getAccountUid());

    updateCurrentBlock(buddy, account->getContext());

    auto self = shared_from_this();              
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
    }).template flatMap<Unit>(account->getContext(), [self, buddy] (auto) {
        return self->synchronizeMempool(buddy);
            })
        .map<tezos::AccountSynchronizationContext>(
            ImmediateExecutionContext::INSTANCE,
            [=](const Unit&) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                buddy->logger->info(
                    "End synchronization for account#{} of wallet {} in {}",
                    buddy->account->getIndex(),
                    buddy->account->getWallet()->getName(),
                    DurationUtils::formatDuration(duration));

                const auto& batches = buddy->savedState.getValue().batches;

                soci::session sql(buddy->wallet->getDatabase()->getPool());
                buddy->context.lastBlockHeight =
                    BlockDatabaseHelper::getLastBlock(
                        sql, buddy->wallet->getCurrency().name)
                    .map<uint64_t>([](const Block& block) {
                        return block.height;
                    })
                    .getValueOr(0);
                fullSyncBenchmarker->stop();
                self->_currentAccount = nullptr;
                return buddy->context;
            })
        .recover(
            ImmediateExecutionContext::INSTANCE,
            [self, buddy, fullSyncBenchmarker](const Exception& ex) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                buddy->logger->error(
                    "Error during during synchronization for account#{} of wallet {} in {} ms",
                    buddy->account->getIndex(),
                    buddy->account->getWallet()->getName(),
                    duration.count());
                buddy->logger->error("Due to {}, {}", api::to_string(ex.getErrorCode()), ex.getMessage());
                fullSyncBenchmarker->stop();
                return buddy->context;
            });
}

Future<Unit> TezosLikeAccountSynchronizer::synchronizeBatches(
    uint32_t currentBatchIndex,
    std::shared_ptr<tezos::SynchronizationBuddy> buddy)
{
    buddy->logger->info("SYNC BATCHES");
    if (currentBatchIndex >= buddy->savedState.getValue().batches.size()) {
        buddy->savedState.getValue().batches.push_back(tezos::AccountSynchronizationBatchSavedState());
    }

    auto self = shared_from_this();
    auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];

    auto benchmark = std::make_shared<Benchmarker>(
        fmt::format("full_batch/{}", buddy->synchronizationTag),
        buddy->logger);
    benchmark->start();
    return synchronizeBatch(currentBatchIndex, buddy)
        .flatMap<Unit>(
            buddy->account->getContext(),
            [=](const bool& hadTransactions) {
                benchmark->stop();

                buddy->preferences->editor()->putObject<tezos::AccountSynchronizationSavedState>(
                    "state", buddy->savedState.getValue())->commit();
                return Future<Unit>::successful(unit);
            })
        .recoverWith(
            ImmediateExecutionContext::INSTANCE,
            [=](const Exception& exception) {
                buddy->logger->info(
                    "Recovering from failing synchronization : {}", exception.getMessage());

                //A block reorganization happened
                if (exception.getErrorCode() == api::ErrorCode::BLOCK_NOT_FOUND &&
                    buddy->savedState.nonEmpty()) {
                    buddy->logger->info("Recovering from reorganization");

                    const auto deactivateToken = 
                        buddy->configuration->getBoolean(api::Configuration::DEACTIVATE_SYNC_TOKEN).value_or(false);
                    auto startSession = Future<void *>::async(ImmediateExecutionContext::INSTANCE, [=](){
                        if (deactivateToken) {
                            return Future<void *>::successful(nullptr);
                        }
                        return self->_explorer->startSession();
                    });

                    return startSession.flatMap<Unit>(
                        ImmediateExecutionContext::INSTANCE,
                        [=](void * const session) {
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
                            auto& failedBatch = buddy->savedState.getValue().batches[currentBatchIndex];
                            auto const failedBlockHeight = failedBatch.blockHeight;
                            auto const failedBlockHash = failedBatch.blockHash;

                            if (failedBlockHeight > 0) {

                                //Delete data related to failedBlock (and all blocks above it)
                                buddy->logger->info(
                                    "Deleting blocks above block height: {}", failedBlockHeight);

                                soci::session sql(buddy->wallet->getDatabase()->getPool());
                                {
                                    soci::transaction tr(sql);
                                    try {
                                        soci::rowset<std::string> rows_block = (sql.prepare << "SELECT uid FROM blocks where height >= :failedBlockHeight",
                                                                                soci::use(failedBlockHeight));

                                        std::vector<std::string> blockToDelete(rows_block.begin(), rows_block.end());

                                        // Fetch all operations which are deleted during reorganization
                                        auto deletedOperationUIDs = OperationDatabaseHelper::fetchFromBlocks(sql, blockToDelete);

                                        // Remove failed blocks and associated operations/transactions
                                        AccountDatabaseHelper::removeBlockOperation(sql, buddy->account->getAccountUid(), blockToDelete);

                                        //Get last block not part from reorg
                                        auto lastBlock = BlockDatabaseHelper::getLastBlock(sql, buddy->wallet->getCurrency().name);

                                        //Resync from the "beginning" if no last block in DB
                                        int64_t lastBlockHeight = 0;
                                        std::string lastBlockHash;
                                        if (lastBlock.nonEmpty()) {
                                            lastBlockHeight = lastBlock.getValue().height;
                                            lastBlockHash = lastBlock.getValue().blockHash;
                                        }
                                        // update reorganization block height until found the valid one
                                        buddy->context.reorgBlockHeight = lastBlockHeight;

                                        //Update savedState's batches
                                        for (auto& batch : buddy->savedState.getValue().batches) {
                                            if (batch.blockHeight > lastBlockHeight) {
                                                batch.blockHeight = static_cast<uint32_t>(lastBlockHeight);
                                                batch.blockHash = lastBlockHash;
                                            }
                                        }
                                        tr.commit();

                                        // We can emit safely deleted operation UIDs
                                        std::for_each(
                                            deletedOperationUIDs.cbegin(),
                                            deletedOperationUIDs.cend(),
                                            [buddy](auto const& uid) {
                                                buddy->account->emitDeletedOperationEvent(uid);
                                            });
                                    } catch(...) {
                                        tr.rollback();
                                    }
                                }

                                //Save new savedState
                                buddy->preferences->editor()->putObject<tezos::AccountSynchronizationSavedState>(
                                        "state", buddy->savedState.getValue())->commit();

                                //Synchronize same batch now with an existing block (of hash lastBlockHash)
                                //if failedBatch was not the deepest block part of that reorg, this recursive call
                                //will ensure to get (and delete from DB) to the deepest failed block (part of reorg)
                                buddy->logger->info("Relaunch synchronization after recovering from reorganization");

                                return self->synchronizeBatches(currentBatchIndex, buddy);
                            }
                            return Future<Unit>::successful(unit);
                        })
                    .recover(
                        ImmediateExecutionContext::INSTANCE,
                        [buddy](const Exception& ex) {
                            buddy->logger->warn(
                                    "Failed to recover from reorganisation for account#{} of wallet {}",
                                    buddy->account->getIndex(),
                                    buddy->account->getWallet()->getName());
                            return unit;
                        });
                }
                return Future<Unit>::successful(unit);
            });
}

Future<bool> TezosLikeAccountSynchronizer::synchronizeBatch(
    uint32_t currentBatchIndex,
    std::shared_ptr<tezos::SynchronizationBuddy> buddy,
    bool hadTransactions)
{
    buddy->logger->info("SYNC BATCH {}", currentBatchIndex);

    Option<std::string> blockHash;
    auto self = shared_from_this();
    auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];

    if (batchState.blockHeight > 0) {
        blockHash = Option<std::string>(batchState.blockHash);
    }


    auto derivationBenchmark = std::make_shared<Benchmarker>(
        fmt::format("derivations/{}", buddy->synchronizationTag),
        buddy->logger);
    derivationBenchmark->start();

    auto batch = vector::map<std::string, std::shared_ptr<TezosLikeAddress>>(
        buddy->keychain->getAllObservableAddresses(
            static_cast<uint32_t>((currentBatchIndex * buddy->halfBatchSize)),
            static_cast<uint32_t>((currentBatchIndex + 1) * buddy->halfBatchSize - 1)),
        [](const std::shared_ptr<TezosLikeAddress>& addr) {
            return addr->toString();
        });

    derivationBenchmark->stop();

    auto benchmark = std::make_shared<Benchmarker>(
        fmt::format("explorer_calls/{}", buddy->synchronizationTag),
        buddy->logger);
    benchmark->start();
    return _explorer->getTransactions(batch, blockHash, buddy->token)
        .flatMap<bool>(
            buddy->account->getContext(),
            [self, currentBatchIndex, buddy, hadTransactions, benchmark](
                const std::shared_ptr<typename TezosLikeBlockchainExplorer::TransactionsBulk>& bulk) {
                benchmark->stop();

                auto interpretBenchmark = std::make_shared<Benchmarker>(
                    fmt::format("interpret_operations/{}", buddy->synchronizationTag),
                    buddy->logger);

                auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];
                //self->transactions.insert(self->transactions.end(), bulk->transactions.begin(), bulk->transactions.end());
                buddy->logger->info("Got {} txs for account {}", bulk->transactions.size(), buddy->account->getAccountUid());
                auto count = 0;

                // NEW CODE
                Option<Block> lastBlock = Option<Block>::NONE;
                std::vector<Operation> operations;
                bool addedNewAddressInBatch = false;
                interpretBenchmark->start();
                // Interpret transactions to operations and update last block
                for (const auto& tx : bulk->transactions) {
                    // Update last block to chain query
                    if (lastBlock.isEmpty() || (tx.block.nonEmpty() &&
                        lastBlock.getValue().height < tx.block.getValue().height)) {
                        lastBlock = tx.block;
                    }

                    /*
                     * This call goes to TezosLikeAccount::interpretTransaction, which might add new originatedAccounts
                     * through updateOriginatedAccounts. This might be an issue because at that point the buddy won't check
                     * the rest of this batch for other transactions associated to its originated account.
                     * I.e., if the TransactionBulk that contains the originated account goes from block 800 to block 900,
                     * in block 840 for example, then all operations for that originated account between 841 and 900 would be missed, as
                     * the synchronizer buddy would only synchronize starting at 90 afterwards
                     */
                    addedNewAddressInBatch = buddy->account->interpretTransaction(tx, operations);

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
                }
                interpretBenchmark->stop();
                auto insertionBenchmark = std::make_shared<Benchmarker>(
                    fmt::format("insert_operations/{}", buddy->synchronizationTag),
                    buddy->logger);
                insertionBenchmark->start();
                Try<int> tryPutTx = buddy->account->bulkInsert(operations);
                insertionBenchmark->stop();
                if (tryPutTx.isFailure()) {
                    buddy->logger->error(
                        "Failed to bulk insert for batch {} because: {}",
                        currentBatchIndex, tryPutTx.getFailure().getMessage());
                    throw make_exception(
                        api::ErrorCode::RUNTIME_ERROR,
                        "Synchronization failed for batch {} ({})",
                        currentBatchIndex, tryPutTx.exception().getValue().getMessage());
                } else {
                    count += tryPutTx.getValue();
                }

                buddy->logger->info(
                    "Succeeded to insert {} txs on {} for account {}",
                    count, bulk->transactions.size(), buddy->account->getAccountUid());
                buddy->account->emitEventsNow();

                // END NEW CODE

                // if an address has been added to the keychain during this batch, we skip buddy state update
                // The goal is to restart the synchronization of the same batch with the extra knowledge of the address
                if (!addedNewAddressInBatch) {
                    if (bulk->transactions.size() > 0 && lastBlock.nonEmpty()) {
                        batchState.blockHeight = static_cast<uint32_t>(lastBlock.getValue().height);
                        batchState.blockHash = lastBlock.getValue().hash;
                        buddy->preferences->editor()->putObject<tezos::AccountSynchronizationSavedState>(
                            "state", buddy->savedState.getValue())->commit();
                    }
                }

                auto hadTX = hadTransactions || bulk->transactions.size() > 0;
                if (bulk->hasNext || addedNewAddressInBatch) {
                    return self->synchronizeBatch(currentBatchIndex, buddy, hadTX);
                } else {
                    return Future<bool>::successful(hadTX);
                }
            });
}

Future<Unit> TezosLikeAccountSynchronizer::synchronizeMempool(
    const std::shared_ptr<tezos::SynchronizationBuddy>& buddy)
{
    //Delete dropped txs from DB
    soci::session sql(buddy->wallet->getDatabase()->getPool());
    for (auto& tx : buddy->transactionsToDrop) {
        //Check if tx is pending
        auto it = buddy->savedState.getValue().pendingTxsHash.find(tx.first);
        if (it == buddy->savedState.getValue().pendingTxsHash.end()) {
            //soci::transaction tr(sql);
            buddy->logger->info("Drop transaction {}", tx.first);
            buddy->logger->info("Deleting operation from DB {}", tx.second);
            try {
                sql << "DELETE FROM operations WHERE uid = :uid", soci::use(tx.second);
                //tr.commit();
            } catch(std::exception& ex) {
                buddy->logger->info(
                    "Failed to delete operation from DB {} reason: {}, rollback ...",
                    tx.second, ex.what());
                //tr.rollback();
            }
        }
    }
    return Future<Unit>::successful(unit);
}

void TezosLikeAccountSynchronizer::updateCurrentBlock(
    std::shared_ptr<tezos::SynchronizationBuddy>& buddy,
    const std::shared_ptr<api::ExecutionContext>& context)
{
    _explorer->getCurrentBlock().onComplete(
        context, [buddy](const TryPtr<TezosLikeBlockchainExplorer::Block>& block) {
            if (block.isSuccess()) {
                soci::session sql(buddy->account->getWallet()->getDatabase()->getPool());
                soci::transaction tr(sql);
                try {
                    buddy->account->putBlock(sql, *block.getValue());
                    tr.commit();
                } catch(...) {
                    tr.rollback();
                }
            }
        });
}

void TezosLikeAccountSynchronizer::updateTransactionsToDrop(
    soci::session& sql,
    std::shared_ptr<tezos::SynchronizationBuddy>& buddy,
    const std::string& accountUid)
{
    //Get all transactions in DB that may be dropped (txs without block_uid)
    soci::rowset<soci::row> rows = (sql.prepare << "SELECT op.uid, xtz_op.transaction_hash FROM operations AS op "
        "LEFT OUTER JOIN tezos_operations AS xtz_op ON xtz_op.uid = op.uid "
        "WHERE op.block_uid IS NULL AND op.account_uid = :uid ", soci::use(accountUid));

    for (auto &row : rows) {
        if (row.get_indicator(0) != soci::i_null && row.get_indicator(1) != soci::i_null) {
            buddy->transactionsToDrop.insert(std::pair<std::string, std::string>(row.get<std::string>(1), row.get<std::string>(0)));
        }
    }
}

} // namespace core
} // namespace ledger
