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
#include <wallet/tezos/synchronizers/TezosLikeAccountSynchronizer.hpp>
#include <wallet/tezos/keychains/TezosLikeKeychain.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseEntry.h>

namespace ledger {
    namespace core {

        class TezosOperationQuery : public OperationQuery {
        public:
            TezosOperationQuery(const std::shared_ptr<api::QueryFilter>& headFilter,
                                const std::shared_ptr<DatabaseSessionPool>& pool,
                                const std::shared_ptr<api::ExecutionContext>& context,
                                const std::shared_ptr<api::ExecutionContext>& mainContext) : OperationQuery(headFilter, pool, context, mainContext) {

            };
        protected:
            virtual soci::rowset<soci::row> performExecute(soci::session &sql) {
                return _builder.select("o.account_uid, o.uid, o.wallet_uid, o.type, o.date, o.senders, o.recipients,"
                                               "o.amount, o.fees, o.currency_name, o.trust, b.hash, b.height, b.time, orig_op.uid"
                        )
                        .from("operations").to("o")
                        .outerJoin("blocks AS b", "o.block_uid = b.uid")
                        .outerJoin("tezos_originated_operations AS orig_op", "o.uid = orig_op.uid")
                        .execute(sql);

            };
        };

        class TezosLikeOriginatedAccount;
        class TezosLikeAccount : public api::TezosLikeAccount, public AbstractAccount {
        public:

            TezosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                             int32_t index,
                             const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                             const std::shared_ptr<TezosLikeAccountSynchronizer> &synchronizer,
                             const std::shared_ptr<TezosLikeKeychain> &keychain);

            void inflateOperation(Operation &out,
                                  const std::shared_ptr<const AbstractWallet> &wallet,
                                  const TezosLikeBlockchainExplorerTransaction &tx);

            /// Return "true" if interpreting this Transaction added a new address in the keychain.
            /// Most common case is if the Transaction is a contract origination.
            bool interpretTransaction(const TezosLikeBlockchainExplorerTransaction& transaction,
                                      std::vector<Operation>& out);

            Try<int> bulkInsert(const std::vector<Operation>& operations);

            /// Return "true" if interpreting this operation added a new address in the keychain.
            /// Most common case is if the Transaction is a contract origination.
            bool updateOriginatedAccounts(const Operation &operation);

            bool putBlock(soci::session& sql, const TezosLikeBlockchainExplorer::Block &block);

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

            std::string getRestoreKey() override;

            void broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                         const std::shared_ptr<api::StringCallback> &callback) override;

            void _broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                         const std::shared_ptr<api::StringCallback> &callback,
                                         const std::shared_ptr<BigInt>& counter,
                                         const std::string& correlationId) ;

            void broadcastTransaction(const std::shared_ptr<api::TezosLikeTransaction> &transaction,
                                      const std::shared_ptr<api::StringCallback> &callback) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder> buildTransaction() override;
            std::shared_ptr<api::TezosLikeTransactionBuilder> buildTransaction(const std::string &senderAddress);

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            void getEstimatedGasLimit(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) override;
            FuturePtr<BigInt> getEstimatedGasLimit(const std::string &address);

            void getStorage(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) override;
            FuturePtr<BigInt> getStorage(const std::string &address);

            std::vector<std::shared_ptr<api::TezosLikeOriginatedAccount>> getOriginatedAccounts() override;

            void addOriginatedAccounts(soci::session &sql, const std::vector<TezosLikeOriginatedAccountDatabaseEntry> &originatedEntries);

            void getFees(const std::shared_ptr<api::BigIntCallback> & callback) override;
            FuturePtr<BigInt> getFees();

            void getGasPrice(const std::shared_ptr<api::BigIntCallback> & callback) override;
            FuturePtr<BigInt> getGasPrice();

            FuturePtr<GasLimit> estimateGasLimit(const std::shared_ptr<TezosLikeTransactionApi>& tx, double adjustment_factor = 1.1);

            std::shared_ptr<api::Keychain> getAccountKeychain() override;

            void incrementOptimisticCounter(std::shared_ptr<TezosLikeTransactionApi> tx, const std::shared_ptr<BigInt>& explorerCounter);

            void saveOptimisticCounter(const std::shared_ptr<BigInt>& counter, const std::string& txHash);

            void getCurrentDelegate(const std::shared_ptr<api::StringCallback> & callback) override;
            Future<std::string> getCurrentDelegate();

            void getTokenBalance(const std::string& tokenAddress, const std::shared_ptr<api::BigIntCallback>& callback) override;
        private:
            std::shared_ptr<TezosLikeAccount> getSelf();
            void broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                         const std::shared_ptr<api::StringCallback> &callback,
                                         const std::string& correlationId);

            std::shared_ptr<TezosLikeKeychain> _keychain;
            std::string _accountAddress;
            std::shared_ptr<TezosLikeBlockchainExplorer> _explorer;
            std::shared_ptr<TezosLikeAccountSynchronizer> _synchronizer;
            std::shared_ptr<api::EventBus> _currentSyncEventBus;
            std::mutex _synchronizationLock;
            std::vector<std::shared_ptr<api::TezosLikeOriginatedAccount>> _originatedAccounts;
            uint64_t _currentBlockHeight;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEACCOUNT_H
