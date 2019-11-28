/*
 *
 * TezosLikeAccount
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

#pragma once

#include <time.h>

#include <tezos/api/TezosLikeAccount.hpp>
#include <tezos/api/TezosLikeTransactionBuilder.hpp>
#include <tezos/explorers/TezosLikeBlockchainExplorer.hpp>
#include <tezos/synchronizers/TezosLikeAccountSynchronizer.hpp>
#include <tezos/observers/TezosLikeBlockchainObserver.hpp>
#include <tezos/keychains/TezosLikeKeychain.hpp>
#include <tezos/database/TezosLikeAccountDatabaseEntry.hpp>
#include <tezos/operations/TezosLikeOperation.hpp>

#include <core/api/Address.hpp>
#include <core/api/Event.hpp>
#include <core/wallet/AbstractWallet.hpp>
#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/Amount.hpp>
#include <core/operation/OperationQuery.hpp>

namespace ledger {
    namespace core {

        class TezosLikeOriginatedAccount;
        class TezosLikeAccount : public api::TezosLikeAccount, public AbstractAccount {
        public:
            static const int FLAG_TRANSACTION_IGNORED = 0x00;

            TezosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                             int32_t index,
                             const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                             const std::shared_ptr<TezosLikeBlockchainObserver> &observer,
                             const std::shared_ptr<TezosLikeAccountSynchronizer> &synchronizer,
                             const std::shared_ptr<TezosLikeKeychain> &keychain);

            void inflateOperation(TezosLikeOperation &operation,
                                  const std::shared_ptr<const AbstractWallet> &wallet,
                                  const TezosLikeBlockchainExplorerTransaction &tx);

            int putTransaction(soci::session &sql,
                               const TezosLikeBlockchainExplorerTransaction &transaction,
                               const std::string &originatedAccountUid = "",
                               const std::string &originatedAccountAddress = "");

            void updateOriginatedAccounts(soci::session &sql, const TezosLikeOperation &operation);

            bool putBlock(soci::session &sql, const api::Block &block);

            std::shared_ptr<TezosLikeKeychain> getKeychain() const;

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

            void broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                         const std::function<void(std::experimental::optional<std::string>, std::experimental::optional<::ledger::core::api::Error>)>& callback) override;

            void broadcastTransaction(const std::shared_ptr<api::TezosLikeTransaction> &transaction,
                                      const std::function<void(std::experimental::optional<std::string>, std::experimental::optional<::ledger::core::api::Error>)>& callback) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder> buildTransaction() override;
            std::shared_ptr<api::TezosLikeTransactionBuilder> buildTransaction(const std::string &senderAddress);

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            void getEstimatedGasLimit(
                const std::string & address,
                const std::function<void(std::experimental::optional<std::shared_ptr<api::BigInt>>, std::experimental::optional<api::Error>)> & callback) override;
            FuturePtr<BigInt> getEstimatedGasLimit(const std::string &address);

            void getStorage(
                const std::string & address,
                const std::function<void(std::experimental::optional<std::shared_ptr<api::BigInt>>, std::experimental::optional<api::Error>)> & callback) override;

            FuturePtr<BigInt> getStorage(const std::string &address);

            std::vector<std::shared_ptr<api::TezosLikeOriginatedAccount>> getOriginatedAccounts() override;

            void addOriginatedAccounts(soci::session &sql, const std::vector<TezosLikeOriginatedAccountDatabaseEntry> &originatedEntries);

            void getFees(
                const std::function<void(std::experimental::optional<std::shared_ptr<api::BigInt>>, std::experimental::optional<api::Error>)> & callback) override;
            
            FuturePtr<BigInt> getFees();

        private:
            std::shared_ptr<TezosLikeAccount> getSelf();

            std::shared_ptr<TezosLikeKeychain> _keychain;
            std::string _accountAddress;
            std::shared_ptr<TezosLikeBlockchainExplorer> _explorer;
            std::shared_ptr<TezosLikeAccountSynchronizer> _synchronizer;
            std::shared_ptr<TezosLikeBlockchainObserver> _observer;
            std::shared_ptr<api::EventBus> _currentSyncEventBus;
            std::mutex _synchronizationLock;
            std::vector<std::shared_ptr<api::TezosLikeOriginatedAccount>> _originatedAccounts;
        };
    }
}
