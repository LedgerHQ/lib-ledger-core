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


#ifndef LEDGER_CORE_TEZOSLIKEACCOUNT_H
#define LEDGER_CORE_TEZOSLIKEACCOUNT_H
#include <time.h>
#include <api/AddressListCallback.hpp>
#include <api/Address.hpp>
#include <api/TezosLikeAccount.hpp>
#include <api/TezosLikeTransactionBuilder.hpp>
#include <api/StringCallback.hpp>
#include <api/Event.hpp>
#include <api/BigIntCallback.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/Amount.h>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/tezos/synchronizers/TezosLikeAccountSynchronizer.h>
#include <wallet/tezos/observers/TezosLikeBlockchainObserver.h>
#include <wallet/tezos/keychains/TezosLikeKeychain.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseEntry.h>
//#include <wallet/tezos/delegation/TezosLikeOriginatedAccount.h>

namespace ledger {
    namespace core {
        class TezosLikeOriginatedAccount;
        class TezosLikeAccount : public api::TezosLikeAccount, public AbstractAccount {
        public:

            static const int FLAG_TRANSACTION_IGNORED = 0x00;
            static const int FLAG_NEW_TRANSACTION = 0x01;
            static const int FLAG_TRANSACTION_UPDATED = 0x01 << 1;

            TezosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                             int32_t index,
                             const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                             const std::shared_ptr<TezosLikeBlockchainObserver> &observer,
                             const std::shared_ptr<TezosLikeAccountSynchronizer> &synchronizer,
                             const std::shared_ptr<TezosLikeKeychain> &keychain,
                             const std::vector<TezosLikeOriginatedAccountDatabaseEntry> &originatedAccounts = std::vector<TezosLikeOriginatedAccountDatabaseEntry>());

            void inflateOperation(Operation &out,
                                  const std::shared_ptr<const AbstractWallet> &wallet,
                                  const TezosLikeBlockchainExplorerTransaction &tx);

            int putTransaction(soci::session &sql,
                               const TezosLikeBlockchainExplorerTransaction &transaction,
                               const std::string &originatedAccountUid = "",
                               const std::string &originatedAccountAddress = "");

            void updateOriginatedAccounts(soci::session &sql, const Operation &operation);

            bool putBlock(soci::session &sql, const TezosLikeBlockchainExplorer::Block &block);

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
                                         const std::shared_ptr<api::StringCallback> &callback) override;

            void broadcastTransaction(const std::shared_ptr<api::TezosLikeTransaction> &transaction,
                                      const std::shared_ptr<api::StringCallback> &callback) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder> buildTransaction() override;

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            void getEstimatedGasLimit(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) override ;

            void getStorage(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) override;

            void recoverOriginatedAccounts();

            std::vector<std::shared_ptr<api::TezosLikeOriginatedAccount>> getOriginatedAccounts() override;

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
            std::vector<TezosLikeOriginatedAccountDatabaseEntry> _originatedAccountsEntries;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEACCOUNT_H
