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
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <async/algorithm.h>

namespace ledger {
    namespace core {

        constexpr auto ADDRESS_BATCH_SIZE = 10;

        using Transaction = BitcoinLikeBlockchainExplorerTransaction;
        using Input = BitcoinLikeBlockchainExplorerInput;
        using CommonBuddy = BlockchainExplorerAccountSynchronizer::SynchronizationBuddy;
        using TransactionMap = std::unordered_map<std::string, std::pair<Transaction, bool /* replaceable */>>;

        struct BitcoinSynchronizationBuddy : public CommonBuddy {
            std::vector<BitcoinLikeBlockchainExplorerTransaction> mempoolTransaction;
            std::vector<BitcoinLikeBlockchainExplorerTransaction> previousMempool;
        };

        BlockchainExplorerAccountSynchronizer::BlockchainExplorerAccountSynchronizer(
                const std::shared_ptr<WalletPool> &pool,
                const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer) :
                DedicatedContext(pool->getDispatcher()
                                     ->getThreadPoolExecutionContext("synchronizers")) {
            _explorer = explorer;
        }


        void BlockchainExplorerAccountSynchronizer::updateCurrentBlock(std::shared_ptr<SynchronizationBuddy> &buddy,
                                                                       const std::shared_ptr<api::ExecutionContext> &context) {
            _explorer->getCurrentBlock().onComplete(context, [buddy] (const TryPtr<BitcoinLikeBlockchainExplorer::Block>& block) {
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

        void BlockchainExplorerAccountSynchronizer::updateTransactionsToDrop(soci::session &sql,
                                                                             std::shared_ptr<SynchronizationBuddy> &buddy,
                                                                             const std::string &accountUid) {
            //Get all transactions in DB that may be dropped (txs without block_uid)
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT op.uid, btc_op.transaction_hash FROM operations AS op "
                                                            "LEFT OUTER JOIN bitcoin_operations AS btc_op ON btc_op.uid = op.uid "
                                                            "WHERE op.block_uid IS NULL AND op.account_uid = :uid ", soci::use(accountUid));

            // Remove all mempool transaction
            BitcoinLikeTransactionDatabaseHelper::removeAllMempoolOperation(sql, accountUid);
            auto btcBuddy = std::static_pointer_cast<BitcoinSynchronizationBuddy>(buddy);
            // Get all operation in mempool (will be used to restore mempool in case of error)
            BitcoinLikeTransactionDatabaseHelper::getMempoolTransactions(sql, accountUid, btcBuddy->previousMempool);

            for (auto &row : rows) {
                if (row.get_indicator(0) != soci::i_null && row.get_indicator(1) != soci::i_null) {
                    buddy->transactionsToDrop.insert(std::pair<std::string, std::string>(row.get<std::string>(1), row.get<std::string>(0)));
                }
            }

        }


        void BlockchainExplorerAccountSynchronizer::reset(const std::shared_ptr<BitcoinLikeAccount>& account,
                                                          const std::chrono::system_clock::time_point& toDate) {

        }

        std::shared_ptr<ProgressNotifier<BlockchainExplorerAccountSynchronizationResult>> BlockchainExplorerAccountSynchronizer::synchronize(const std::shared_ptr<BitcoinLikeAccount>& account) {
            return synchronizeAccount(account);
        }

        bool BlockchainExplorerAccountSynchronizer::isSynchronizing() const {
            return _notifier != nullptr;
        }

        std::shared_ptr<BlockchainExplorerAccountSynchronizer> BlockchainExplorerAccountSynchronizer::getSharedFromThis() {
            return shared_from_this();
        }

        std::shared_ptr<api::ExecutionContext> BlockchainExplorerAccountSynchronizer::getSynchronizerContext() {
            return getContext();
        }

        void BlockchainExplorerAccountSynchronizer::interpretTransaction(const Transaction& transaction,
                                                                        const std::shared_ptr<SynchronizationBuddy>& buddy,
                                                                        std::vector<Operation>& out) {
            // In case the transaction is in mempool, keep in memory before inserting it in database.
            if (transaction.block.isEmpty()) {
                std::static_pointer_cast<BitcoinSynchronizationBuddy>(buddy)->mempoolTransaction.emplace_back(
                        transaction);
                auto keychain = buddy->account->getKeychain();
                auto const markAddress = [&keychain](auto &txIo) {
                    auto flag = 0;

                    for (auto &io : txIo) {
                        if (io.address.hasValue()) {
                            keychain->markAsUsed(io.address.getValue(), false);
                            flag |= BitcoinLikeAccount::FLAG_TRANSACTION_CREATED_SENDING_OPERATION;
                        }
                    }
                    return flag;
                };
                markAddress(transaction.inputs);
                markAddress(transaction.outputs);
            } else {
                buddy->account->interpretTransaction(transaction, out);
            }
        }

        std::shared_ptr<CommonBuddy>
        BlockchainExplorerAccountSynchronizer::makeSynchronizationBuddy() {
            return std::make_shared<BitcoinSynchronizationBuddy>();
        }

        inline bool isInputReplaced(const Input& input, uint32_t conflictingInputSequence) {
            return input.sequence < conflictingInputSequence;
        }

        inline bool isInputReplaceable(const Input& input) {
            return input.sequence < std::numeric_limits<uint32_t>::max();
        }

        void filterReplacedTransaction(const std::vector<Transaction>& transactions, TransactionMap& txByHash) {
            using InputWithTxHash = std::tuple<std::string /* tx hash */, uint32_t /* input sequence */>;
            std::unordered_map<std::string, InputWithTxHash> txByIntputs;
            for (const auto& tx : transactions) {
                if (txByHash.find(tx.hash) != txByHash.end()) {
                    // Transaction is already in result map, don't go though all filtering.
                    continue;
                }
                bool replaceable = false;
                bool replaced = false;
                for (const auto& input : tx.inputs) {
                    if (!input.previousTxHash || !input.previousTxOutputIndex)
                        continue;
                    auto inputId = fmt::format("{}{}", input.previousTxHash.getValue(), input.previousTxOutputIndex.getValue());
                    auto entry = txByIntputs.find(inputId);
                    if (entry == txByIntputs.end()) {
                        txByIntputs[inputId] = std::make_pair(tx.hash, input.sequence);
                    } else if (!isInputReplaced(input, std::get<1>(entry->second))) {
                        replaced = replaced || (std::get<0>(entry->second) == tx.hash);
                        txByHash.erase(std::get<0>(entry->second));
                        txByIntputs[inputId] = std::make_pair(tx.hash, input.sequence);
                    } else {
                        replaced = true;
                        break;
                    }
                    replaceable = replaceable || isInputReplaceable(input);
                }
                if (!replaced)
                    txByHash[tx.hash] = std::make_pair(tx, replaceable);
            }
        }

        Future<std::vector<Transaction>> getMempoolTransactionBatch(const std::shared_ptr<CommonBuddy>& buddy,
                                                                    const api::Block& lastBlock,
                                                                    const std::vector<std::string>& addresses) {
            return buddy->account->getExplorer()->getTransactions(addresses, Option<std::string>(lastBlock.blockHash))
            .map<std::vector<Transaction>>(buddy->account->getContext(), [] (const auto& bulk) {
                std::vector<Transaction> mempoolTxs;
                for (const auto& tx : bulk->transactions) {
                    if (!tx.block.hasValue())
                        mempoolTxs.emplace_back(tx);
                }
                return mempoolTxs;
            });
        }

        Future<std::vector<Transaction>> getMempoolTransactions(const std::shared_ptr<CommonBuddy>& buddy,
                                                                const std::vector<std::string>& addresses) {
            using TransactionBatchVector = std::vector<std::vector<Transaction>>;
            return buddy->account->getLastBlock()
            .flatMap<TransactionBatchVector>(buddy->account->getContext(), [=] (const auto& block) {
                std::vector<Future<std::vector<Transaction>>> getTxs;
                for(std::size_t i = 0; i < addresses.size(); i += ADDRESS_BATCH_SIZE) {
                    auto last = std::min(addresses.size(), i + ADDRESS_BATCH_SIZE);
                    std::vector<std::string> batch(addresses.begin() + i, addresses.begin() + last);
                    getTxs.emplace_back(getMempoolTransactionBatch(buddy, block, batch));
                }
                return async::sequence(buddy->account->getContext(), getTxs);
            }).map<std::vector<Transaction>>(buddy->account->getContext(), [] (const auto& batches) {
                std::vector<Transaction> mempoolTxs;
                for (const auto& batch : batches) {
                    mempoolTxs.insert(mempoolTxs.end(), batch.begin(), batch.end());
                }
                return mempoolTxs;
            });
        }

        Future<Unit> resolveMempool(const std::shared_ptr<BitcoinSynchronizationBuddy>& buddy) {
            // Do a first round of filtering with transaction bound to the account
            TransactionMap txsByHash;
            filterReplacedTransaction(buddy->mempoolTransaction, txsByHash);
            // Get all transaction which are still replaceable and create a list of addresses
            std::vector<Transaction> notReplaceableTxs;
            std::vector<Transaction> replaceableTxs;
            std::unordered_set<std::string> foreignAddresses;

            for (const auto& txWithHash : txsByHash) {
                if (txWithHash.second.second) {
                    // Tx is replaceable add it in the list to filter later
                    // and go though all inputs to find foreign address
                    replaceableTxs.emplace_back(txWithHash.second.first);
                    for (const auto& input : txWithHash.second.first.inputs) {
                        if (input.address.hasValue() &&
                            !buddy->account->getKeychain()->contains(input.address.getValue())) {
                            foreignAddresses.insert(input.address.getValue());
                        }
                    }
                } else {
                    notReplaceableTxs.emplace_back(txWithHash.second.first);
                }
            }
            // Use the list of addresses to get all mempool transaction for those addresses
            return getMempoolTransactions(buddy,
                    std::vector<std::string>(foreignAddresses.begin(), foreignAddresses.end()))
            .map<Unit>(buddy->account->getContext(), [=] (const auto& foreignTxs) {
                // Do another round of filtering on replaceable transaction vs foreign transactions
                TransactionMap filteredTxs;
                std::vector<Transaction> txs(replaceableTxs.begin(), replaceableTxs.end());
                txs.insert(txs.end(), foreignTxs.begin(), foreignTxs.end());
                filterReplacedTransaction(txs, filteredTxs);
                // Insert everything in database
                std::vector<Operation> operations;
                // Insert not replaceable transactions
                for (const auto& tx : notReplaceableTxs) {
                    buddy->account->interpretTransaction(tx, operations);
                }
                // Insert replaceable transaction
                for (const auto& entry : filteredTxs) {
                    buddy->account->interpretTransaction(entry.second.first, operations);
                }
                buddy->account->bulkInsert(operations);
                return unit;
            });
        }

        Future<Unit> BlockchainExplorerAccountSynchronizer::synchronizeMempool(
                const std::shared_ptr<SynchronizationBuddy> &buddy) {
            auto bitcoinBuddy = std::static_pointer_cast<BitcoinSynchronizationBuddy>(buddy);
            return resolveMempool(bitcoinBuddy);
        }

        Future<Unit> BlockchainExplorerAccountSynchronizer::recoverFromFailedSynchronization(
                const std::shared_ptr<CommonBuddy> &commonBuddy) {
            auto buddy = std::static_pointer_cast<BitcoinSynchronizationBuddy>(commonBuddy);
            return Future<Unit>::async(buddy->account->getContext(), [buddy] () {
                std::vector<Operation> operations;
                for (const auto& tx : buddy->previousMempool) {
                    buddy->account->interpretTransaction(tx, operations);
                }
                buddy->account->bulkInsert(operations);
                return unit;
            });
        }

        void BlockchainExplorerAccountSynchronizer::initializeSavedState(Option<BlockchainExplorerAccountSynchronizationSavedState>& savedState,
            int32_t halfBatchSize) {
            if (savedState.hasValue() && savedState.getValue()
                .halfBatchSize != halfBatchSize) {
                BlockchainExplorerAccountSynchronizationBatchSavedState block;
                block.blockHeight = 1U << 31U;

                for (auto& state : savedState.getValue()
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
                    .halfBatchSize = (uint32_t)halfBatchSize;
                for (auto i = 0; i <= newBatchCount; i++) {
                    BlockchainExplorerAccountSynchronizationBatchSavedState s;
                    s.blockHash = block.blockHash;
                    s.blockHeight = block.blockHeight;
                    savedState.getValue().batches.push_back(s);
                }
            }
            else if (savedState.isEmpty()) {
                savedState = Option<BlockchainExplorerAccountSynchronizationSavedState>(
                    BlockchainExplorerAccountSynchronizationSavedState());
                savedState.getValue()
                    .halfBatchSize = (uint32_t)halfBatchSize;
            }
        };

        std::shared_ptr<ProgressNotifier<BlockchainExplorerAccountSynchronizationResult>> BlockchainExplorerAccountSynchronizer::synchronizeAccount(const std::shared_ptr<BitcoinLikeAccount>& account) {
            std::lock_guard<std::mutex> lock(_lock);
            if (!_currentAccount) {
                _currentAccount = account;
                _notifier = std::make_shared<ProgressNotifier<BlockchainExplorerAccountSynchronizationResult>>();
                auto self = getSharedFromThis();
                performSynchronization(account).onComplete(getSynchronizerContext(), [self](auto const& result) {
                    std::lock_guard<std::mutex> l(self->_lock);
                    if (result.isFailure()) {
                        self->_notifier->failure(result.getFailure());
                    }
                    else {
                        self->_notifier->success(result.getValue());
                    }
                    self->_notifier = nullptr;
                    self->_currentAccount = nullptr;
                    });

            }
            else if (account != _currentAccount) {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "This synchronizer is already in use");
            }
            return _notifier;
        };


        Future<BlockchainExplorerAccountSynchronizationResult> BlockchainExplorerAccountSynchronizer::performSynchronization(const std::shared_ptr<BitcoinLikeAccount>& account)
        {
            auto buddy = makeSynchronizationBuddy();
            buddy->account = account;
            buddy->preferences = std::static_pointer_cast<AbstractAccount>(account)->getInternalPreferences()
                ->getSubPreferences("AbstractBlockchainExplorerAccountSynchronizer");
            auto loggerPurpose = fmt::format("synchronize_{}", account->getAccountUid());
            auto tracePrefix = fmt::format("{}/{}/{}", account->getWallet()->getPool()->getName(), account->getWallet()->getName(), account->getIndex());
            buddy->logger = logger::trace(loggerPurpose, tracePrefix, account->logger());
            buddy->startDate = DateUtils::now();
            buddy->wallet = account->getWallet();
            buddy->configuration = std::static_pointer_cast<AbstractAccount>(account)->getWallet()->getConfig();
            buddy->halfBatchSize = (uint32_t)buddy->configuration
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
                std::sort(sortedBatches.begin(), sortedBatches.end(), [](const BlockchainExplorerAccountSynchronizationBatchSavedState& lhs,
                    const BlockchainExplorerAccountSynchronizationBatchSavedState& rhs) -> bool {
                        return lhs.blockHeight < rhs.blockHeight;
                    });

                auto currencyName = buddy->wallet->getCurrency().name;
                size_t index = 0;
                //Reorg can't happen until genesis block, safely initialize with 0
                uint64_t deepestFailedBlockHeight = 0;
                while (index < sortedBatches.size() && !BlockDatabaseHelper::blockExists(sql, sortedBatches[index].blockHash, currencyName)) {
                    deepestFailedBlockHeight = sortedBatches[index].blockHeight;
                    index++;
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
            self->_addresses.clear();
            //self->transactions.clear();
            return self->extendKeychain(0, buddy).template flatMap<Unit>(account->getContext(), [buddy, self](const Unit&) {
                return self->synchronizeBatches(0, buddy);
                }).template flatMap<Unit>(account->getContext(), [self, buddy](auto) {
                    return self->synchronizeMempool(buddy);
                    }).template map<BlockchainExplorerAccountSynchronizationResult>(ImmediateExecutionContext::INSTANCE, [self, buddy](const Unit&) {
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                        buddy->logger->info("End synchronization for account#{} of wallet {} in {}", buddy->account->getIndex(),
                            buddy->account->getWallet()->getName(), DurationUtils::formatDuration(duration));

                        auto const& batches = buddy->savedState.getValue().batches;

                        // get the last block height treated during the synchronization
                        // std::max_element returns an iterator hence the indirection here
                        // We use an constant iterator variable for readability purpose
                        auto const batchIt = std::max_element(
                            std::cbegin(batches),
                            std::cend(batches),
                            [](auto const& lhs, auto const& rhs) {
                                return lhs.blockHeight < rhs.blockHeight;
                            });
                        soci::session sql(buddy->wallet->getDatabase()->getPool());
                        buddy->context.lastBlockHeight = BlockDatabaseHelper::getLastBlock(sql,
                            buddy->wallet->getCurrency().name).template map<uint64_t>([](const Block& block) {
                                return block.height;
                                }).getValueOr(0);

                                self->_currentAccount = nullptr;
                                return buddy->context;
                        }).recover(ImmediateExecutionContext::INSTANCE, [self, buddy](const Exception& ex) {
                            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                                (DateUtils::now() - buddy->startDate.time_since_epoch()).time_since_epoch());
                            buddy->logger->error("Error during during synchronization for account#{} of wallet {} in {} ms", buddy->account->getIndex(),
                                buddy->account->getWallet()->getName(), duration.count());
                            buddy->logger->error("Due to {}, {}", api::to_string(ex.getErrorCode()), ex.getMessage());
                            return buddy->context;
                            });
        };

        // extend the keychain to cover all the addresses(only for bitcoin)
        Future<Unit> BlockchainExplorerAccountSynchronizer::extendKeychain(uint32_t currentBatchIndex, std::shared_ptr<SynchronizationBuddy> buddy) {
            buddy->logger->info("Detecting addresses for batch {}", currentBatchIndex);
            auto self = getSharedFromThis();
            auto from = currentBatchIndex / 2 * buddy->halfBatchSize;
            auto to = (currentBatchIndex + 1) * buddy->halfBatchSize - 1;
            buddy->logger->info("From address index {}", from);
            buddy->logger->info("To address index {}", to);
            auto batch = buddy->keychain->getAllObservableAddressString(from, to);
            auto lastAddressesinBatch = std::vector<std::string>(batch.end() - 2 * buddy->halfBatchSize, batch.end());
            self->_addresses.insert(self->_addresses.end(), batch.begin(), batch.end());

            return _explorer->getTransactions(lastAddressesinBatch, optional<std::string>(), optional<void*>())
                .template flatMap<Unit>(buddy->account->getContext(), [self, currentBatchIndex, buddy, to](const std::shared_ptr<BitcoinLikeBlockchainExplorer::TransactionsBulk>& bulk) -> Future<Unit> {
                if (bulk->transactions.size() > 0)
                {
                    return self->extendKeychain((currentBatchIndex + 1) * 2, buddy);
                }
                else
                {
                    return Future<Unit>::successful(unit);
                }
                    });
        };


        // Synchronize batches.
        //
        // This function will synchronize all batches by iterating over batches and transactions
        // bulks. The input buddy can be used to customize the behavior of the synchronization.
        Future<Unit> BlockchainExplorerAccountSynchronizer::synchronizeBatches(uint32_t currentBatchIndex, std::shared_ptr<SynchronizationBuddy> buddy) {
            buddy->logger->info("SYNC BATCHES");
            auto done = currentBatchIndex >= buddy->savedState.getValue().batches.size() - 1;
            if (currentBatchIndex >= buddy->savedState.getValue().batches.size()) {
                buddy->savedState.getValue().batches.push_back(BlockchainExplorerAccountSynchronizationBatchSavedState());
            }

            auto self = getSharedFromThis();
            auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];

            return synchronizeBatch(currentBatchIndex, buddy).template flatMap<Unit>(buddy->account->getContext(), [=](const bool& hadTransactions) -> Future<Unit> {

                buddy->preferences->editor()->template putObject<BlockchainExplorerAccountSynchronizationSavedState>("state", buddy->savedState.getValue())->commit();

                //Sync stops if there are no more batches in savedState and last batch has no transactions
                //But we may want to force sync of accounts within KEYCHAIN_OBSERVABLE_RANGE
                auto discoveredAddresses = currentBatchIndex * buddy->halfBatchSize;
                auto lastDiscoverableAddress = buddy->configuration->getInt(api::Configuration::KEYCHAIN_OBSERVABLE_RANGE).value_or(buddy->halfBatchSize);
                if (!done || (done && hadTransactions) || lastDiscoverableAddress > discoveredAddresses) {
                    return self->synchronizeBatches(currentBatchIndex + 1, buddy);
                }

                return Future<Unit>::successful(unit);
                }).recoverWith(ImmediateExecutionContext::INSTANCE, [=](const Exception& exception) -> Future<Unit> {
                    buddy->logger->info("Recovering from failing synchronization : {}", exception.getMessage());
                    //A block reorganization happened
                    if (exception.getErrorCode() == api::ErrorCode::BLOCK_NOT_FOUND &&
                        buddy->savedState.nonEmpty()) {
                        buddy->logger->info("Recovering from reorganization");
                        auto startSession = Future<void*>::async(ImmediateExecutionContext::INSTANCE, [=]() {
                            return Future<void*>::successful(nullptr);
                            });

                        return startSession.template flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=](void* const session) {
                            //Get its block/block height
                            auto& failedBatch = buddy->savedState.getValue().batches[currentBatchIndex];
                            auto const failedBlockHeight = failedBatch.blockHeight;
                            auto const failedBlockHash = failedBatch.blockHash;

                            if (failedBlockHeight > 0) {

                                //Delete data related to failedBlock (and all blocks above it)
                                buddy->logger->info("Deleting blocks above block height: {}", failedBlockHeight);

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
                                        auto lastBlock = BlockDatabaseHelper::getLastBlock(sql,
                                            buddy->wallet->getCurrency().name);

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
                                                batch.blockHeight = (uint32_t)lastBlockHeight;
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
                                    }
                                    catch (...) {
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
                            }).recover(ImmediateExecutionContext::INSTANCE, [buddy](const Exception& ex) -> Unit {
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
        // whether more data is needed. If a block doesn�t have any transaction, it means that
        // we must stop.
        Future<bool> BlockchainExplorerAccountSynchronizer::synchronizeBatch(uint32_t currentBatchIndex, std::shared_ptr<SynchronizationBuddy> buddy, bool hadTransactions) {
            buddy->logger->info("SYNC BATCH {}", currentBatchIndex);

            Option<std::string> blockHash;
            auto self = getSharedFromThis();
            auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];

            if (batchState.blockHeight > 0) {
                blockHash = Option<std::string>(batchState.blockHash);
            }


            int from = currentBatchIndex * buddy->halfBatchSize * 2;
            int to = (currentBatchIndex + 1) * buddy->halfBatchSize * 2;
            auto batch = std::vector<std::string>(self->_addresses.begin() + from, min(self->_addresses.begin() + to, self->_addresses.end()));

            return _explorer->getTransactions(batch, blockHash, optional<void*>())
                .template flatMap<bool>(buddy->account->getContext(), [self, currentBatchIndex, buddy, hadTransactions](const std::shared_ptr<BitcoinLikeBlockchainExplorer::TransactionsBulk>& bulk) -> Future<bool> {

                auto& batchState = buddy->savedState.getValue().batches[currentBatchIndex];
                //self->transactions.insert(self->transactions.end(), bulk->transactions.begin(), bulk->transactions.end());
                buddy->logger->info("Got {} txs for account {}", bulk->transactions.size(), buddy->account->getAccountUid());
                auto count = 0;
                for (const auto& tx : bulk->transactions) {
                    soci::session sql(buddy->wallet->getDatabase()->getPool());
                    soci::transaction tr(sql);
                    // A lot of things could happen here, better to wrap it	
                    auto tryPutTx = Try<int>::from([&buddy, &tx, &sql, &self]() {
                        auto const flag = self->putTransaction(sql, tx, buddy);

                        if (::ledger::core::account::isInsertedOperation(flag)) {
                            ++buddy->context.newOperations;
                        }

                        //Update first pendingTxHash in savedState	
                        auto it = buddy->transactionsToDrop.find(tx.hash);
                        if (it != buddy->transactionsToDrop.end()) {
                            //If block non empty, tx is no longer pending	
                            if (tx.block.nonEmpty()) {
                                buddy->savedState.getValue().pendingTxsHash.erase(it->first);
                            }
                            else { //Otherwise tx is in mempool but pending	
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
                        throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Synchronization failed for batch {} on block {} because of tx {} ({})", currentBatchIndex, blockHash, tx.hash, tryPutTx.exception().getValue().getMessage());
                    }
                    else {
                        count++;
                        tr.commit();
                    }
                }
                buddy->logger->info("Succeeded to insert {} txs on {} for account {}", count, bulk->transactions.size(), buddy->account->getAccountUid());
                buddy->account->emitEventsNow();

                // Get the last block
                if (bulk->transactions.size() > 0) {
                    auto& lastBlock = bulk->transactions.back().block;

                    if (lastBlock.nonEmpty()) {
                        batchState.blockHeight = (uint32_t)lastBlock.getValue().height;
                        batchState.blockHash = lastBlock.getValue().hash;
                        buddy->preferences->editor()->template putObject<BlockchainExplorerAccountSynchronizationSavedState>("state", buddy->savedState.getValue())->commit();
                    }
                }

                auto hadTX = hadTransactions || bulk->transactions.size() > 0;
                return Future<bool>::successful(hadTX);                    
                });
        };
    }
}
