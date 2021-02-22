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

#include "TezosLikeAccountSynchronizer.hpp"

#include <async/algorithm.h>
#include <utils/DateUtils.hpp>
#include <utils/ImmediateExecutionContext.hpp>
#include <wallet/tezos/TezosLikeAccount.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/delegation/TezosLikeOriginatedAccount.h>

#include <algorithm>
#include <numeric>

namespace ledger {
namespace core {

TezosLikeAccountSynchronizer::TezosLikeAccountSynchronizer(
    const std::shared_ptr<WalletPool>& pool,
    const std::shared_ptr<TezosLikeBlockchainExplorer>& explorer)
    : DedicatedContext(pool->getDispatcher()->getThreadPoolExecutionContext("synchronizers"))
    , _explorer(explorer)
    , _account(nullptr)
    , _notifier(nullptr)
    , _logger(nullptr)
    , _m()
{}

auto TezosLikeAccountSynchronizer::synchronize(
    const std::shared_ptr<TezosLikeAccount>& account)
    -> std::shared_ptr<ProgressNotifier<Result>>
{
  std::lock_guard<std::mutex> lock(_m);
  if (!_account) {
      _account = account;
      _notifier = std::make_shared<ProgressNotifier<Result>>();
      _logger = logger::trace(
          fmt::format("synchronize_{}", account->getAccountUid()),
          fmt::format("{}/{}/{}", account->getWallet()->getName(),
                      account->getWallet()->getName(), account->getIndex()),
          account->logger());
      _start = DateUtils::now();

      performSynchronization().onComplete(getContext(), [this](const Try<Result>& result) {
          std::lock_guard<std::mutex> lock(_m);
          if (result.isFailure()) {
              _notifier->failure(result.getFailure());
          } else {
              _notifier->success(result.getValue());
          }
          _account = nullptr;
          _notifier = nullptr;
          _logger = nullptr;
      });
  }
  return _notifier;
}

auto TezosLikeAccountSynchronizer::performSynchronization() const -> Future<Result>
{
    const auto state = std::make_shared<SavedState>(_account
        ->getInternalPreferences()
        ->getSubPreferences("TezosLikeAccountSynchronizerr")
        ->getObject<SavedState>("state").getValueOr(SavedState()));


    _logger->info(
        "Starting synchronization for account#{} ({}) of wallet {} at {}.",
        _account->getIndex(),
        _account->getKeychain()->getRestoreKey(),
        _account->getWallet()->getName(),
        DateUtils::toJSON(_start));

    const auto block = updateCurrentBlock();

    const auto syncs = [this, state]() {
        const auto originatedAccounts = _account->getOriginatedAccounts();
        auto syncs = std::vector<Future<uint32_t>>();
        // synchronization of all XTZ transactions
        syncs.push_back(synchronizeTransactions<false>(
            _account->getKeychain()->getAddress()->toString(), state));
        // synchronization of all originated accounts
        std::transform(
            std::begin(originatedAccounts),
            std::end(originatedAccounts),
            std::back_inserter(syncs),
            [this, state](const auto& account) {
                return synchronizeTransactions<true>(account->getAddress(), state);
            });

        return syncs;
    }();

    auto futures = std::make_tuple(block, async::sequence(getContext(), syncs));
    return async::sequence(getContext(), futures)
        .map<Result>(
            getContext(),
            [this, state](const auto& tuple) {
                const auto duration = std::chrono::duration_cast<
                    std::chrono::milliseconds>(DateUtils::now() - _start);
                _logger->info(
                    "End synchronization of account#{} of wallet {} in {}.",
                    _account->getIndex(),
                    _account->getWallet()->getName(),
                    DurationUtils::formatDuration(duration));

                const auto block = std::get<0>(tuple);
                const auto syncs = std::get<1>(tuple);

                const auto newOperations = std::accumulate(
                    std::begin(syncs), std::end(syncs), 0);

                if (block.height > 0) {
                    _account->getInternalPreferences()
                            ->editor()
                            ->putObject<SavedState>("state", *state)
                            ->commit();
                }

                return Result(block.height, newOperations);
            })
        .recoverWith(
            ImmediateExecutionContext::INSTANCE,
            [this](const Exception& ex) {
                const auto duration = std::chrono::duration_cast<
                    std::chrono::milliseconds>(DateUtils::now() - _start);
                _logger->error(
                    "Error during synchronization for account#{} of wallet {} in {}.",
                    _account->getIndex(),
                    _account->getWallet()->getName(),
                    DurationUtils::formatDuration(duration));
                _logger->error(
                    "Due to {}, {}.",
                    api::to_string(ex.getErrorCode()),
                    ex.getMessage());

                return Future<Result>::failure(ex);
            });
}

template<bool orig>
auto TezosLikeAccountSynchronizer::synchronizeTransactions(
    const std::string& address,
    const std::shared_ptr<SavedState>& state,
    uint32_t nbTxns) const -> Future<uint32_t>
{
    const auto uid = [this, address]() {
        if (orig) {
            return TezosLikeAccountDatabaseHelper::createOriginatedAccountUid(
                _account->getAccountUid(), address);
        }
        return std::string("");
    }();
    return _explorer->getTransactions({address}, state->getOffset(address))
        .flatMap<uint32_t>(
            _account->getContext(),
            [this, address, state, uid, nbTxns](
                const std::shared_ptr<TezosLikeBlockchainExplorer::TransactionsBulk>& bulk)
            {
                auto out = std::vector<Operation>();
                for (auto& tx : bulk->transactions) {
                    if (tx.block.hasValue() && tx.block->height > state->blockHeight) {
                        state->blockHeight = tx.block->height;
                        state->blockHash = tx.block->hash;
                    }

                    if (orig) {
                        tx.originatedAccountUid = uid;
                        tx.originatedAccountAddress = address;
                    }
                    _account->interpretTransaction(tx, out);
                }

                const auto tryPutTx = _account->bulkInsert(out);
                if (tryPutTx.isFailure()) {
                    _logger->error(
                        "Insert transaction bulk failed beacause: {}",
                        tryPutTx.getFailure().getMessage());

                    throw make_exception(
                        api::ErrorCode::RUNTIME_ERROR,
                        "Synchronization failed ({})",
                        tryPutTx.exception().getValue().getMessage());
                }

                const auto count = tryPutTx.getValue();
                state->addOffset(address, count);

                _logger->info(
                    "Successfully inserted {} transactions for account {}.",
                    count, _account->getAccountUid());

                if (bulk->hasNext) {
                    return synchronizeTransactions<orig>(address, state, nbTxns + count);
                } else {
                    return Future<uint32_t>::successful(nbTxns + count);
                }
            });
}

auto TezosLikeAccountSynchronizer::updateCurrentBlock() const -> Future<Block>
{
    return _explorer->getCurrentBlock()
        .map<Block>(
            _account->getContext(),
            [this](const std::shared_ptr<TezosLikeBlockchainExplorer::Block>& block) {
                if (block) {
                    soci::session sql(_account->getWallet()->getDatabase()->getPool());
                    soci::transaction tr(sql);
                    try {
                        _account->putBlock(sql, *block);
                        tr.commit();
                    } catch(...) {
                        tr.rollback();
                    }
                    return *block;
                }
                return Block();
            });
}

} // namespace core
} // namespace ledger
