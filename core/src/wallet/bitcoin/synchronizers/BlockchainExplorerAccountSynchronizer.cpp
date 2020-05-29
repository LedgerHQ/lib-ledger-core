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

        void BlockchainExplorerAccountSynchronizer::updateCurrentBlock(std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
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

        std::shared_ptr<BlockchainAccountSynchronizer> BlockchainExplorerAccountSynchronizer::getSharedFromThis() {
            return shared_from_this();
        }

        std::shared_ptr<api::ExecutionContext> BlockchainExplorerAccountSynchronizer::getSynchronizerContext() {
            return getContext();
        }

        int BlockchainExplorerAccountSynchronizer::putTransaction(soci::session &sql,
                                                                  const BitcoinLikeBlockchainExplorerTransaction &transaction,
                                                                  const std::shared_ptr<CommonBuddy> &buddy) {
            // In case the transaction is in mempool, keep in memory before inserting it in database.
            if (transaction.block.isEmpty()) {
                std::static_pointer_cast<BitcoinSynchronizationBuddy>(buddy)->mempoolTransaction.emplace_back(
                        transaction);
                auto keychain = buddy->account->getKeychain();
                auto const markAddress = [&keychain](auto &txIo) {
                    auto flag = 0;

                    for (auto &io : txIo) {
                        if (io.address.hasValue()) {
                            keychain->markAsUsed(io.address.getValue());
                            flag |= BitcoinLikeAccount::FLAG_TRANSACTION_CREATED_SENDING_OPERATION;
                        }
                    }
                    return flag;
                };
                return markAddress(transaction.inputs) | markAddress(transaction.outputs);
            }
            return buddy->account->putTransaction(sql, transaction);
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
                soci::session sql(buddy->account->getWallet()->getDatabase()->getPool());
                soci::transaction tr(sql);
                // Insert not replaceable transactions
                for (const auto& tx : notReplaceableTxs) {
                    buddy->account->putTransaction(sql, tx);
                }
                // Insert replaceable transaction
                for (const auto& entry : filteredTxs) {
                    buddy->account->putTransaction(sql, entry.second.first);
                }
                tr.commit();
                return unit;
            });
        }

        Future<Unit> BlockchainExplorerAccountSynchronizer::synchronizeMempool(
                const std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer<BitcoinLikeAccount, BitcoinLikeAddress, BitcoinLikeKeychain, BitcoinLikeBlockchainExplorer>::SynchronizationBuddy> &buddy) {
            auto bitcoinBuddy = std::static_pointer_cast<BitcoinSynchronizationBuddy>(buddy);
            return resolveMempool(bitcoinBuddy);
        }

        Future<Unit> BlockchainExplorerAccountSynchronizer::recoverFromFailedSynchronization(
                const std::shared_ptr<CommonBuddy> &commonBuddy) {
            auto buddy = std::static_pointer_cast<BitcoinSynchronizationBuddy>(commonBuddy);
            return Future<Unit>::async(buddy->account->getContext(), [buddy] () {
                soci::session sql(buddy->account->getWallet()->getDatabase()->getPool());
                for (const auto& tx : buddy->previousMempool) {
                    soci::transaction tr(sql);
                    auto result = make_try<int>([&] () {
                        return buddy->account->putTransaction(sql, tx);
                    });
                    if (result.isSuccess()) {
                        tr.commit();
                    } else {
                        tr.rollback();
                        buddy->logger->error("Unable to restore transaction {} ({})", tx.hash, result.getFailure().getMessage());
                    }
                }
                return unit;
            });
        }

    }
}
