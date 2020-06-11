/*
 *
 * AlgorandAccountSynchronizer
 *
 * Created by Hakim Aammar on 01/06/2020
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "AlgorandAccountSynchronizer.hpp"
#include "AlgorandAccount.hpp"

#include <wallet/common/database/AccountDatabaseHelper.h>
#include <utils/DateUtils.hpp>
#include <utils/DurationUtils.h>
#include <collections/vector.hpp>
#include <debug/Benchmarker.h>
#include <api/Configuration.hpp>
#include <api/ConfigurationDefaults.hpp>

namespace ledger {
namespace core {
namespace algorand {

    AccountSynchronizer::AccountSynchronizer(const std::shared_ptr<WalletPool> & pool,
                                             const std::shared_ptr<BlockchainExplorer> & explorer) :
        DedicatedContext(pool->getDispatcher()->getThreadPoolExecutionContext("synchronizers")),
        _explorer(explorer)
    {}

    std::shared_ptr<ProgressNotifier<Unit>> AccountSynchronizer::synchronizeAccount(const std::shared_ptr<Account>& account) {
        std::lock_guard<std::mutex> lock(_lock);

        if (!_account) {
            _account = account;
            _notifier = std::make_shared<ProgressNotifier<Unit>>();

            performSynchronization(account)
                .onComplete(getContext(), [this] (const Try<Unit> &result) {
                    std::lock_guard<std::mutex> l(_lock);
                    if (result.isFailure()) {
                        _notifier->failure(result.getFailure());
                    } else {
                        _notifier->success(unit);
                    }
                    _notifier = nullptr;
                    _account = nullptr;
                });

        } else if (account != _account) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "This synchronizer is already in use");
        }
        return _notifier;
    };

    Future<Unit> AccountSynchronizer::performSynchronization(const std::shared_ptr<Account> & account) {

        _internalPreferences = account->getInternalPreferences()->getSubPreferences("AlgorandAccountSynchronizer");
        auto savedState = _internalPreferences->template getObject<SavedState>("state");
        auto startRound = Option<uint64_t>();
        if (savedState.hasValue()) {
            startRound = savedState.getValue().round;
        } else {
            savedState = Option<SavedState>(SavedState());
            savedState.getValue().round = 0;
            _internalPreferences->editor()->template putObject<SavedState>("state", savedState.getValue())->commit();
        }

        return synchronizeBatch(account, startRound, false)
            .template flatMap<Unit>(account->getContext(), [] (const bool hadTransactions) -> Future<Unit> {
                return Future<Unit>::successful(unit);
            }).recoverWith(ImmediateExecutionContext::INSTANCE, [] (const Exception & exception) -> Future<Unit> {
                return Future<Unit>::failure(exception);
            });
    }

    Future<bool> AccountSynchronizer::synchronizeBatch(const std::shared_ptr<Account> & account,
                                                       const Option<uint64_t> & maxRound,
                                                       const bool hadTransactions) {
        return _explorer->getTransactionsForAddress(account->getAddress(), maxRound)
            .template flatMap<bool>(getContext(), [this, account, hadTransactions](const model::TransactionsBulk& bulk) -> Future<bool> {

                soci::session sql(account->getWallet()->getDatabase()->getPool());
                soci::transaction tr(sql);

                auto lowestRound = std::numeric_limits<uint64_t>::max();
                auto highestRound = static_cast<uint64_t>(0);
                for (const auto& tx : bulk.transactions) {

                    // A lot of things could happen here, better wrap it
                    auto tryPutTx = Try<int>::from([&] () {
                        return account->putTransaction(sql, tx);
                    });

                    // Record the lowest round, to continue fetching txs below it if needed
                    lowestRound = std::min(lowestRound, tx.header.round.getValueOr(lowestRound));

                    // Record the highest round, to update the cache for incremental sychronization
                    highestRound = std::max(highestRound, tx.header.round.getValueOr(0));
                }

                tr.commit();

                auto savedState = _internalPreferences->template getObject<SavedState>("state");
                if (savedState.hasValue()) {
                    savedState.getValue().round = std::max(savedState.getValue().round, highestRound);
                    _internalPreferences->editor()->template putObject<SavedState>("state", savedState.getValue())->commit();
                }

                auto hadTX = hadTransactions || bulk.transactions.size() > 0;
                if (bulk.hasNext) {
                    return synchronizeBatch(account, lowestRound, hadTX);
                } else {
                    return Future<bool>::successful(hadTX);
                }
            });
    }

} // namespace algorand
} // namespace core
} // namespace ledger

