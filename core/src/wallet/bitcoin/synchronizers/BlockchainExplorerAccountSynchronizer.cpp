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

namespace ledger {
    namespace core {

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

            for (auto &row : rows) {
                if (row.get_indicator(0) != soci::i_null && row.get_indicator(1) != soci::i_null) {
                    buddy->transactionsToDrop.insert(std::pair<std::string, std::string>(row.get<std::string>(1), row.get<std::string>(0)));
                }
            }

        }


        void BlockchainExplorerAccountSynchronizer::reset(const std::shared_ptr<BitcoinLikeAccount>& account,
                                                          const std::chrono::system_clock::time_point& toDate) {

        }

        std::shared_ptr<ProgressNotifier<Unit>> BlockchainExplorerAccountSynchronizer::synchronize(const std::shared_ptr<BitcoinLikeAccount>& account) {
            std::lock_guard<std::mutex> lock(_lock);
            if (!_currentAccount) {
                _currentAccount = account;
                _notifier = std::make_shared<ProgressNotifier<Unit>>();
                auto self = std::dynamic_pointer_cast<BlockchainExplorerAccountSynchronizer>(getSharedFromThis());
                performSynchronization(account)
                .flatMap<std::vector<std::string>>(getSynchronizerContext(), [self] (auto unit) {
                    return self->getReplacedTransactionHashes();
                }).map<Unit>(getSynchronizerContext(), [self] (const auto& hashes) {
                    self->_currentAccount->dropTransactions(hashes);
                    return unit;
                }).onComplete(getSynchronizerContext(), [self] (const auto &result) {
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

        std::shared_ptr<BlockchainAccountSynchronizer> BlockchainExplorerAccountSynchronizer::getSharedFromThis() {
            return shared_from_this();
        }

        std::shared_ptr<api::ExecutionContext> BlockchainExplorerAccountSynchronizer::getSynchronizerContext() {
            return getContext();
        }

        Future<std::vector<std::string>> BlockchainExplorerAccountSynchronizer::getReplacedTransactionHashes() {
            // Get all transaction from the mempool

            // If two (or more) transactions share the same input group them
            // (and mark them as RBF transactions)

            // Find the highest input sequence of the group and remove all
            // transaction using a sequence which is less than the max in database

            // For all transaction marked as RBF, iterate through all inputs
            // of all transaction to get addresses

            // Get all transactions in mempool for the addresses

            // Iterate through all transaction and remove from database
            // my RBF transactions if the sequence is higher in one of the addresses transactions
            return Future<std::vector<std::string>>::successful({});
        }

    }
}