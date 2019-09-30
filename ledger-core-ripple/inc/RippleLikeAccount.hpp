/*
 *
 * RippleLikeAccount
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

#pragma once

#include <time.h>
#include <core/api/Address.hpp>
#include <core/api/Event.hpp>
#include <core/wallet/AbstractWallet.hpp>
#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/Amount.hpp>
#include <api/RippleLikeAccount.hpp>
#include <api/RippleLikeTransactionBuilder.hpp>
#include <explorers/RippleLikeBlockchainExplorer.hpp>
#include <observers/RippleLikeBlockchainObserver.hpp>
#include <keychains/RippleLikeKeychain.hpp>
#include <synchronizers/RippleLikeAccountSynchronizer.hpp>

namespace ledger {
    namespace core {
        class RippleLikeAccount : public api::RippleLikeAccount, public AbstractAccount {
        public:
            static const int FLAG_TRANSACTION_IGNORED = 0x00;
            static const int FLAG_NEW_TRANSACTION = 0x01;
            static const int FLAG_TRANSACTION_UPDATED = 0x01 << 1;

            RippleLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                              int32_t index,
                              const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer,
                              const std::shared_ptr<RippleLikeBlockchainObserver> &observer,
                              const std::shared_ptr<RippleLikeAccountSynchronizer> &synchronizer,
                              const std::shared_ptr<RippleLikeKeychain> &keychain);

            FuturePtr<RippleLikeBlockchainExplorerTransaction> getTransaction(const std::string &hash);

            void inflateOperation(Operation &out,
                                  const std::shared_ptr<const AbstractWallet> &wallet,
                                  const RippleLikeBlockchainExplorerTransaction &tx);

            int putTransaction(soci::session &sql, const RippleLikeBlockchainExplorerTransaction &transaction);

            bool putBlock(soci::session &sql, const RippleLikeBlockchainExplorer::Block &block);

            std::shared_ptr<RippleLikeKeychain> getKeychain() const;

            FuturePtr<Amount> getBalance() override;

            Future<AbstractAccount::AddressList> getFreshPublicAddresses() override;

            Future<std::vector<std::shared_ptr<api::Amount>>>
            getBalanceHistory(const std::string &start,
                              const std::string &end,
                              api::TimePeriod precision) override;

            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point &date) override;

            bool isSynchronizing() override;

            std::shared_ptr<api::EventBus> synchronize() override;

            void startBlockchainObservation() override;

            void stopBlockchainObservation() override;

            bool isObservingBlockchain() override;

            std::string getRestoreKey() override;

            void broadcastRawTransaction(
                const std::vector<uint8_t> & transaction,
                const std::function<void(std::experimental::optional<std::string>, std::experimental::optional<api::Error>)> & callback
            ) override;

            void broadcastTransaction(
                const std::shared_ptr<api::RippleLikeTransaction> & transaction,
                const std::function<void(std::experimental::optional<std::string>, std::experimental::optional<api::Error>)> & callback
            ) override;

            std::shared_ptr<api::RippleLikeTransactionBuilder> buildTransaction() override;

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            void getFees(const std::function<void(std::experimental::optional<std::shared_ptr<api::Amount>>, std::experimental::optional<api::Error>)> & callback) override;
            FuturePtr<api::Amount> getFees();

            void getBaseReserve(const std::function<void(std::experimental::optional<std::shared_ptr<api::Amount>>, std::experimental::optional<api::Error>)> & callback) override;
            FuturePtr<api::Amount> getBaseReserve();
        private:
            std::shared_ptr<RippleLikeAccount> getSelf();

            std::shared_ptr<RippleLikeKeychain> _keychain;
            std::string _accountAddress;
            std::shared_ptr<RippleLikeBlockchainExplorer> _explorer;
            std::shared_ptr<RippleLikeAccountSynchronizer> _synchronizer;
            std::shared_ptr<RippleLikeBlockchainObserver> _observer;
            std::shared_ptr<api::EventBus> _currentSyncEventBus;
            std::mutex _synchronizationLock;
            uint64_t _currentLedgerSequence;
        };
    }
}
