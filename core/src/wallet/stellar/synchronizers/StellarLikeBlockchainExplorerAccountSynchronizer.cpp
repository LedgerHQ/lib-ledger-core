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

        std::shared_ptr<ProgressNotifier<Unit>> StellarLikeBlockchainExplorerAccountSynchronizer::synchronize(
                const std::shared_ptr<StellarLikeAccount> &account) {
            if (_notifier != nullptr) {
                return _notifier;
            }
            _notifier = std::make_shared<ProgressNotifier<Unit>>();
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
                    ->template getObject<StellarLikeBlockchainExplorerAccountSynchronizer::SavedState>("state");
            synchronizeAccount(account, state);
        }

        void
        StellarLikeBlockchainExplorerAccountSynchronizer::synchronizeAccount(const std::shared_ptr<StellarLikeAccount>& account,
                                                                             const Option<StellarLikeBlockchainExplorerAccountSynchronizer::SavedState> &state) {
            auto address = account->getKeychain()->getAddress()->toString();
            auto self = shared_from_this();
            _explorer->getAccount(address).onComplete(account->getContext(), [=] (const Try<std::shared_ptr<stellar::Account>>& acc) {
                if (acc.isFailure() && acc.getFailure().getErrorCode() == api::ErrorCode::ACCOUNT_NOT_FOUND) {
                    self->endSynchronization();
                } else if (acc.isFailure()) {
                    self->failSynchronization(acc.getFailure());
                } else {
                    soci::session sql(_database->getPool());
                    account->updateAccountInfo(sql, *acc.getValue());
                   self->synchronizeTransactions(account, state);
                }
            });
        }

        void StellarLikeBlockchainExplorerAccountSynchronizer::synchronizeTransactions(
                const std::shared_ptr<StellarLikeAccount> &account,
                const Option<StellarLikeBlockchainExplorerAccountSynchronizer::SavedState> &state) {
            auto address = account->getKeychain()->getAddress()->toString();
            auto transactionCursor = state.flatMap<std::string>([] (const SavedState& s) {
                return s.transactionPagingToken.empty() ? Option<std::string>() : Option<std::string>(s.transactionPagingToken);
            });

            auto self = shared_from_this();
            _explorer->getTransactions(address, transactionCursor).onComplete(account->getContext(), [=] (const Try<stellar::TransactionVector>& txs) {
                SavedState newState = state.getValueOr(SavedState());
                if (txs.isFailure()) {
                    self->failSynchronization(txs.getFailure());
                } else {
                    {
                        soci::session sql(_database->getPool());
                        soci::transaction tr(sql);

                        for (const auto &tx : txs.getValue()) {
                            account->putTransaction(sql, *tx);
                        }
                        tr.commit();
                    }
                    if (!txs.getValue().empty()) {
                        newState.algorithmVersion = SYNCHRONIZATION_ALGORITHM_VERSION;
                        newState.transactionPagingToken = txs.getValue().back()->pagingToken;
                        auto preferences = account->getInternalPreferences()->getSubPreferences("StellarLikeBlockchainExplorerAccountSynchronizer");
                        preferences->editor()->putObject("state", newState)->commit();
                        self->synchronizeTransactions(account, Option<SavedState>(newState));
                    } else {
                        self->endSynchronization();
                    }
                }
            });
        }

        void StellarLikeBlockchainExplorerAccountSynchronizer::endSynchronization() {
            _notifier->success(unit);
            _notifier = nullptr;
        }


        void StellarLikeBlockchainExplorerAccountSynchronizer::failSynchronization(const Exception &ex) {
            _notifier->failure(ex);
            _notifier = nullptr;
        }

    }
}