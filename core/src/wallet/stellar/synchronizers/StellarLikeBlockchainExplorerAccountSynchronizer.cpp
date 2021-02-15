/*
 *
 * StellarLikeBlockchainExplorerAccountSynchronizer.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/07/2019.
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

#include "StellarLikeBlockchainExplorerAccountSynchronizer.hpp"

#include <common/AccountHelper.hpp>
#include <wallet/stellar/StellarLikeAccount.hpp>

/**
 * Current version of the synchronization. Please keep this field up to date to ensure that we have proper migration
 * mechanism if the synchronizer needs to save more data in its state in the future (or modify current value). This
 * version allow smooth migration.
 */
static const auto SYNCHRONIZATION_ALGORITHM_VERSION = 1;

namespace ledger {
    namespace core {

        StellarLikeBlockchainExplorerAccountSynchronizer::StellarLikeBlockchainExplorerAccountSynchronizer(
                const std::shared_ptr<WalletPool> &pool,
                const std::shared_ptr<StellarLikeBlockchainExplorer> &explorer) :
                DedicatedContext(pool->getDispatcher()->getSerialExecutionContext("stellar_like_account_synchronizer")),
                _explorer(explorer),
                _database(pool->getDatabaseSessionPool()) {

        }

        void StellarLikeBlockchainExplorerAccountSynchronizer::reset(const std::shared_ptr<StellarLikeAccount> &account,
                                                                     const std::chrono::system_clock::time_point &toDate) {
            account->getInternalPreferences()->getSubPreferences("StellarLikeBlockchainExplorerAccountSynchronizer")->editor()->clear();
        }

        std::shared_ptr<ProgressNotifier<BlockchainExplorerAccountSynchronizationResult>> StellarLikeBlockchainExplorerAccountSynchronizer::synchronize(
                const std::shared_ptr<StellarLikeAccount> &account) {
            if (_notifier != nullptr) {
                return _notifier;
            }
            _notifier = std::make_shared<ProgressNotifier<BlockchainExplorerAccountSynchronizationResult>>();
            synchronizeAccount(account);
            return _notifier;
        }

        bool StellarLikeBlockchainExplorerAccountSynchronizer::isSynchronizing() const {
            return _notifier != nullptr;
        }

        void StellarLikeBlockchainExplorerAccountSynchronizer::synchronizeAccount(const std::shared_ptr<StellarLikeAccount> &account) {
            auto self = shared_from_this();
            auto preferences = account->getInternalPreferences()->getSubPreferences("StellarLikeBlockchainExplorerAccountSynchronizer");
            auto state = preferences
                ->template getObject<StellarLikeBlockchainExplorerAccountSynchronizer::SavedState>("state")
                // provide default state if none exists yet
                .getValueOr(SavedState{});

            synchronizeAccount(account, state);
        }

        void
        StellarLikeBlockchainExplorerAccountSynchronizer::synchronizeAccount(const std::shared_ptr<StellarLikeAccount>& account,
                                                                             StellarLikeBlockchainExplorerAccountSynchronizer::SavedState &state) {
            auto address = account->getKeychain()->getAddress()->toString();
            auto self = shared_from_this();

            _explorer->getAccount(address)
                .onComplete(account->getContext(), [self, account, state] (const Try<std::shared_ptr<stellar::Account>>& accountInfo) mutable {
                   if (accountInfo.isFailure() && accountInfo.getFailure().getErrorCode() == api::ErrorCode::ACCOUNT_NOT_FOUND) {
                        self->endSynchronization(account, state);
                    } else if (accountInfo.isFailure()) {
                        self->failSynchronization(accountInfo.getFailure());
                    } else {
                        soci::session sql(self->_database->getPool());
                        account->updateAccountInfo(sql, *accountInfo.getValue());
                        self->synchronizeTransactions(account, state);
                    }
                });
        }

        void StellarLikeBlockchainExplorerAccountSynchronizer::synchronizeTransactions(
                const std::shared_ptr<StellarLikeAccount> &account,
                StellarLikeBlockchainExplorerAccountSynchronizer::SavedState &state) {
            auto address = account->getKeychain()->getAddress()->toString();
            auto self = shared_from_this();
            _explorer->getTransactions(address, state.transactionPagingToken)
                .onComplete(account->getContext(), [self, account, state] (const Try<stellar::TransactionVector>& txs) mutable {
                if (txs.isFailure()) {
                    self->failSynchronization(txs.getFailure());
                } else {
                    {
                        std::vector<Operation> operations;
                        for (const auto &tx : txs.getValue()) {
                            account->logger()->debug("XLM transaction hash: {}, paging_token: {}", tx->hash, tx->pagingToken);
                            account->interpretTransaction(*tx, operations);
                            Try<int> tryPutTx = account->bulkInsert(operations);
                            if (tryPutTx.isFailure()) {
                                account->logger()->error("Failed to bulk insert because: {}", tryPutTx.getFailure().getMessage());
                                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Synchronization failed ({})", tryPutTx.exception().getValue().getMessage());
                            } 
                            else {
                                ++state.insertedOperations;
                            } 
                            
                            state.lastBlockHeight = std::max(state.lastBlockHeight, tx->ledger);
                        }
                    }
                    account->emitEventsNow();
                    if (!txs.getValue().empty()) {
                        state.transactionPagingToken = txs.getValue().back()->pagingToken;
                        {
                            auto preferences = account->getInternalPreferences()->getSubPreferences("StellarLikeBlockchainExplorerAccountSynchronizer");
                            preferences->editor()->putObject("state", state)->commit();
                        }
                       
                        self->synchronizeTransactions(account, state);
                    } else {
                        self->endSynchronization(account, state);
                    }
                }
            });
        }

        void StellarLikeBlockchainExplorerAccountSynchronizer::endSynchronization(
                const std::shared_ptr<StellarLikeAccount>& account,
                const StellarLikeBlockchainExplorerAccountSynchronizer::SavedState &state) {
            BlockchainExplorerAccountSynchronizationResult result;

            soci::session sql(account->getWallet()->getDatabase()->getPool());
            result.lastBlockHeight = BlockDatabaseHelper::getLastBlock(sql,
                                                                               account->getWallet()->getCurrency().name)
                                                                                       .template map<uint64_t>([] (const Block& block) {
                return block.height;
            }).getValueOr(0);

            result.newOperations = state.insertedOperations;


            _notifier->success(result);
            _notifier = nullptr;
        }


        void StellarLikeBlockchainExplorerAccountSynchronizer::failSynchronization(const Exception &ex) {
            _notifier->failure(ex);
            _notifier = nullptr;
        }
    }
}
