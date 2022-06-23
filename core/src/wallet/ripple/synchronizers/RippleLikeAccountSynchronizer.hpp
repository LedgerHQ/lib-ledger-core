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

#ifndef LEDGER_CORE_RIPPLELIKEACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_RIPPLELIKEACCOUNTSYNCHRONIZER_H

#include <async/DedicatedContext.hpp>
#include <events/ProgressNotifier.h>
#include <memory>
#include <mutex>
#include <wallet/pool/WalletPool.hpp>
#include <wallet/ripple/explorers/RippleLikeBlockchainExplorer.h>
#include <wallet/ripple/keychains/RippleLikeKeychain.h>

namespace ledger {
    namespace core {

        class RippleLikeAccount;

        struct AccountSynchronizationBatchSavedState {
            std::string blockHash;
            uint32_t blockHeight;

            AccountSynchronizationBatchSavedState() : blockHash(""), blockHeight(0) {}

            template <class Archive>
            void serialize(Archive &archive) {
                archive(blockHash, blockHeight);
            };
        };

        struct AccountSynchronizationSavedState {
            uint32_t halfBatchSize;
            std::vector<AccountSynchronizationBatchSavedState> batches;
            std::map<std::string, std::string> pendingTxsHash;

            AccountSynchronizationSavedState() : halfBatchSize(0) {}

            template <class Archive>
            void serialize(Archive &archive) {
                archive(halfBatchSize, batches, pendingTxsHash);
            }
        };

        struct AccountSynchronizationContext {
            Option<uint32_t> reorgBlockHeight;
            uint32_t lastBlockHeight = 0;
            uint32_t newOperations   = 0;
        };

        struct SynchronizationBuddy {
            std::shared_ptr<Preferences> preferences;
            std::shared_ptr<spdlog::logger> logger;
            std::chrono::system_clock::time_point startDate;
            std::shared_ptr<AbstractWallet> wallet;
            std::shared_ptr<DynamicObject> configuration;
            uint32_t halfBatchSize;
            std::shared_ptr<RippleLikeKeychain> keychain;
            Option<AccountSynchronizationSavedState> savedState;
            std::shared_ptr<RippleLikeAccount> account;
            std::map<std::string, std::string> transactionsToDrop;
            AccountSynchronizationContext context;
            std::string synchronizationTag;
        };

        class RippleLikeAccountSynchronizer : public DedicatedContext,
                                              public std::enable_shared_from_this<RippleLikeAccountSynchronizer> {
          public:
            RippleLikeAccountSynchronizer(
                const std::shared_ptr<WalletPool> &pool,
                const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer);

            std::shared_ptr<ProgressNotifier<AccountSynchronizationContext>>
            synchronizeAccount(const std::shared_ptr<RippleLikeAccount> &account);

            Future<AccountSynchronizationContext>
            performSynchronization(const std::shared_ptr<RippleLikeAccount> &account);

            Future<Unit> synchronizeBatches(
                uint32_t currentBatchIndex,
                std::shared_ptr<SynchronizationBuddy> buddy);

            Future<bool> synchronizeBatch(
                uint32_t currentBatchIndex,
                std::shared_ptr<SynchronizationBuddy> buddy,
                bool hadTransactions = false);

            Future<Unit> synchronizeMempool(const std::shared_ptr<SynchronizationBuddy> &buddy);

            void updateCurrentBlock(
                std::shared_ptr<SynchronizationBuddy> &buddy,
                const std::shared_ptr<api::ExecutionContext> &context);

            void updateTransactionsToDrop(
                soci::session &sql,
                std::shared_ptr<SynchronizationBuddy> &buddy,
                const std::string &accountUid);

          private:
            std::shared_ptr<RippleLikeBlockchainExplorer> _explorer;
            std::shared_ptr<ProgressNotifier<AccountSynchronizationContext>> _notifier;
            std::mutex _lock;
            std::shared_ptr<RippleLikeAccount> _currentAccount;
            std::shared_ptr<Preferences> _internalPreferences;
        };

    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_RIPPLELIKEACCOUNTSYNCHRONIZER_H
