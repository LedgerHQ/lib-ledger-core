/*
 *
 * CosmosLikeAccountSynchronizer
 *
 * Created by Hakim Aammar on 02/03/2020.
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#ifndef LEDGER_CORE_COSMOSLIKEACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_COSMOSLIKEACCOUNTSYNCHRONIZER_H

#include <algorithm>
#include <memory>
#include <mutex>

#include <core/Services.hpp>
#include <core/api/Configuration.hpp>
#include <core/api/ConfigurationDefaults.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/async/Future.hpp>
#include <core/async/Wait.hpp>
#include <core/collections/DynamicObject.hpp>
#include <core/debug/Benchmarker.hpp>
#include <core/events/ProgressNotifier.hpp>
#include <core/preferences/Preferences.hpp>
#include <core/utils/DateUtils.hpp>
#include <core/utils/DurationUtils.hpp>
#include <core/utils/Unit.hpp>
#include <core/wallet/AbstractWallet.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>
#include <core/wallet/BlockDatabaseHelper.hpp>
#include <cosmos/explorers/BlockchainExplorer.hpp>
#include <cosmos/keychains/Keychain.hpp>

namespace ledger {
namespace core {

namespace cosmos {
class CosmosLikeAccount;
struct AccountSynchronizationBatchSavedState {
  std::string blockHash;
  uint32_t blockHeight;

  AccountSynchronizationBatchSavedState() { blockHeight = 0; }

  template <class Archive> void serialize(Archive &archive) {
    archive(blockHash, blockHeight);
  };
};

struct AccountSynchronizationSavedState {
  uint32_t halfBatchSize;
  std::vector<AccountSynchronizationBatchSavedState> batches;
  std::map<std::string, std::string> pendingTxsHash;

  AccountSynchronizationSavedState() : halfBatchSize(0) {}

  template <class Archive> void serialize(Archive &archive) {
    archive(halfBatchSize, batches,
            pendingTxsHash); // serialize things by passing them to the archive
  }
};

struct SynchronizationBuddy {
  std::shared_ptr<Preferences> preferences;
  std::shared_ptr<spdlog::logger> logger;
  std::chrono::system_clock::time_point startDate;
  std::shared_ptr<AbstractWallet> wallet;
  std::shared_ptr<DynamicObject> configuration;
  uint32_t halfBatchSize;
  std::shared_ptr<CosmosLikeKeychain> keychain;
  Option<cosmos::AccountSynchronizationSavedState> savedState;
  Option<void *> token;
  std::shared_ptr<CosmosLikeAccount> account;
  std::map<std::string, std::string> transactionsToDrop;
};

class CosmosLikeAccountSynchronizer
    : public DedicatedContext,
      public std::enable_shared_from_this<CosmosLikeAccountSynchronizer> {
public:
  CosmosLikeAccountSynchronizer(
      const std::shared_ptr<Services> &services,
      const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer);

  std::shared_ptr<ProgressNotifier<Unit>>
  synchronizeAccount(const std::shared_ptr<CosmosLikeAccount> &account);

  Future<Unit>
  performSynchronization(const std::shared_ptr<CosmosLikeAccount> &account);

  Future<Unit>
  synchronizeBatches(uint32_t currentBatchIndex,
                     std::shared_ptr<cosmos::SynchronizationBuddy> buddy);

  Future<bool>
  synchronizeBatch(uint32_t currentBatchIndex,
                   std::shared_ptr<cosmos::SynchronizationBuddy> buddy,
                   bool hadTransactions = false);

  void
  updateCurrentBlock(std::shared_ptr<cosmos::SynchronizationBuddy> &buddy,
                     const std::shared_ptr<api::ExecutionContext> &context);

  void
  updateTransactionsToDrop(soci::session &sql,
                           std::shared_ptr<cosmos::SynchronizationBuddy> &buddy,
                           const std::string &accountUid);

private:
  std::shared_ptr<Preferences> _internalPreferences;
  std::shared_ptr<DatabaseSessionPool> _database;
  std::shared_ptr<CosmosLikeBlockchainExplorer> _explorer;
  std::shared_ptr<ProgressNotifier<Unit>> _notifier;
  std::mutex _lock;
  std::shared_ptr<CosmosLikeAccount> _currentAccount;
};
} // namespace cosmos
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_COSMOSLIKEACCOUNTSYNCHRONIZER_H
