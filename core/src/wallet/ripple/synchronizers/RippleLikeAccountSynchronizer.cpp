/*
 *
 * RippleLikeAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#include "RippleLikeAccountSynchronizer.hpp"

#include <utils/DateUtils.hpp>
#include <utils/DurationUtils.h>
#include <utils/ImmediateExecutionContext.hpp>
#include <utils/Try.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/ripple/RippleLikeAccount.h>

namespace ledger {
namespace core {

RippleLikeAccountSynchronizer::RippleLikeAccountSynchronizer(
    const std::shared_ptr<WalletPool>& pool,
    const std::shared_ptr<RippleLikeBlockchainExplorer>& explorer)
    : DedicatedContext(pool->getDispatcher()->getThreadPoolExecutionContext("synchronizers"))
    , _explorer(explorer)
    , _notifier(nullptr)
    , _logger(nullptr)
    , _m()
{}

std::shared_ptr<ProgressNotifier<RippleLikeAccountSynchronizer::Result>>
RippleLikeAccountSynchronizer::synchronize(const std::shared_ptr<RippleLikeAccount>& account)
{
    std::lock_guard<std::mutex> lock(_m);
    if (!_notifier) {
        _logger = logger::trace(
            fmt::format("synchronize_{}", account->getAccountUid()),
            fmt::format("{}/{}/{}", account->getWallet()->getName(),
                        account->getWallet()->getName(), account->getIndex()),
            account->logger());

        _notifier = std::make_shared<ProgressNotifier<Result>>();
        _start = DateUtils::now();

        performSynchronization(account).onComplete(getContext(), [this](const Try<Result>& result) {
            std::lock_guard<std::mutex> l(_m);
            if (result.isFailure()) {
                _notifier->failure(result.getFailure());
            } else {
                _notifier->success(result.getValue());
            }
            _notifier = nullptr;
            _logger = nullptr;
        });
    }
    return _notifier;
}

Future<RippleLikeAccountSynchronizer::Result>
RippleLikeAccountSynchronizer::performSynchronization(
    const std::shared_ptr<RippleLikeAccount>& account) const
{
    const auto address = account->getKeychain()->getAddress()->toString();
    const auto preferences = account->getInternalPreferences()->getSubPreferences("RippleLikeAccountSynchronizer");
    auto state = preferences->getObject<SavedState>("state").getValueOr(SavedState{});

    _logger->info(
        "Starting synchronization for account#{} ({}) of wallet {} at {}.",
        account->getIndex(),
        account->getKeychain()->getRestoreKey(),
        account->getWallet()->getName(),
        DateUtils::toJSON(_start));

    updateCurrentBlock(account);

    return synchronizeTransactions(account, state)
        .flatMap<Result>(
            account->getContext(),
            [this, account](uint32_t nbTxns) {
                const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - _start);
                _logger->info(
                    "End synchronization for account#{} of wallet {} in {}.",
                    account->getIndex(),
                    account->getWallet()->getName(),
                    DurationUtils::formatDuration(duration));

                const auto result = [account, nbTxns]() {
                    auto result = Result{};
                    soci::session sql(account->getWallet()->getDatabase()->getPool());
                    const auto lastBlock = BlockDatabaseHelper::getLastBlock(
                        sql, account->getWallet()->getCurrency().name);
                    if (lastBlock.nonEmpty()) {
                        result.lastBlockHeight = lastBlock->height;
                    }
                    result.newOperations = nbTxns;
                    return result;
                }();

                return Future<Result>::successful(result);
            })
        .recoverWith(
            ImmediateExecutionContext::INSTANCE,
            [this, account](const Exception& ex) {
                const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - _start);
                _logger->error(
                    "Error during during synchronization for account#{} of wallet {} in {}.",
                    account->getIndex(),
                    account->getWallet()->getName(),
                    DurationUtils::formatDuration(duration));
                _logger->error(
                    "Due to {}, {}",
                    api::to_string(ex.getErrorCode()),
                    ex.getMessage());

                return Future<Result>::failure(ex);
            });
}

Future<uint32_t> RippleLikeAccountSynchronizer::synchronizeTransactions(
    const std::shared_ptr<RippleLikeAccount>& account, SavedState& state, uint32_t nbTxns) const
{
    const auto address = account->getKeychain()->getAddress()->toString();
    const auto blockHash = state.blockHeight > 0 ? Option<std::string>(state.blockHash) : Option<std::string>();
    return _explorer->getTransactions({address}, blockHash, {})
        .flatMap<uint32_t>(
            account->getContext(),
            [this, account, state, nbTxns](
                const std::shared_ptr<RippleLikeBlockchainExplorer::TransactionsBulk>& bulk) mutable
            {
                std::vector<Operation> out;
                auto lastBlockHeight = state.blockHeight;
                auto lastBlockHash = state.blockHash;
                for (const auto& tx : bulk->transactions) {
                    if (tx.block.nonEmpty() && tx.block->height > lastBlockHeight) {
                        lastBlockHeight = tx.block->height;
                        lastBlockHash = tx.block->hash;
                    }
                    account->interpretTransaction(tx, out);
                }

                const auto tryPutTx = account->bulkInsert(out);
                if (tryPutTx.isFailure()) {
                    _logger->error(
                        "Insert transaction bulk failed because: {}",
                        tryPutTx.getFailure().getMessage());

                    throw make_exception(
                        api::ErrorCode::RUNTIME_ERROR,
                        "Synchronization failed ({})",
                        tryPutTx.exception().getValue().getMessage());
                }

                account->emitEventsNow();

                const auto count = tryPutTx.getValue();
                _logger->info(
                    "Succussfully inserted {} transactions for account {}.",
                    count, account->getAccountUid());

                if (bulk->transactions.size() > 0) {
                    state.blockHeight = lastBlockHeight;
                    state.blockHash = lastBlockHash;
                    account->getInternalPreferences()->editor()->putObject<SavedState>("state", state)->commit();
                }

                if (bulk->hasNext) {
                    return synchronizeTransactions(account, state, nbTxns + count);
                } else {
                    return Future<uint32_t>::successful(nbTxns + count);
                }
            });
}

void RippleLikeAccountSynchronizer::updateCurrentBlock(
    const std::shared_ptr<RippleLikeAccount>& account) const
{
    _explorer->getCurrentBlock()
        .onComplete(
            account->getContext(),
            [account](const TryPtr<RippleLikeBlockchainExplorer::Block> &block) {
                if (block.isSuccess()) {
                    soci::session sql(account->getWallet()->getDatabase()->getPool());
                    soci::transaction tr(sql);
                    try {
                        account->putBlock(sql, *block.getValue());
                        tr.commit();
                    } catch(...) {
                        tr.rollback();
                    }
                }
            });
}

} // namespace core
} // namespace ledger
