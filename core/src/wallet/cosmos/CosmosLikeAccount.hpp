/*
 *
 * CosmosLikeAccount
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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


#ifndef LEDGER_CORE_COSMOSLIKEACCOUNT_H
#define LEDGER_CORE_COSMOSLIKEACCOUNT_H

#include <time.h>

#include <core/api/Address.hpp>
#include <core/api/Event.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/wallet/AbstractWallet.hpp>
#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/Amount.hpp>

#include <cosmos/api/CosmosLikeAccount.hpp>
#include <cosmos/api_impl/CosmosLikeOperation.hpp>
#include <cosmos/api/CosmosLikeTransactionBuilder.hpp>
#include <cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>
#include <cosmos/synchronizers/CosmosLikeAccountSynchronizer.hpp>
#include <cosmos/observers/CosmosLikeBlockchainObserver.hpp>
#include <cosmos/keychains/CosmosLikeKeychain.hpp>

namespace ledger {
        namespace core {
                class CosmosLikeAccount : public api::CosmosLikeAccount, public AbstractAccount {
                        public:
                                static const int FLAG_TRANSACTION_IGNORED = 0x00;

                                CosmosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                                  int32_t index,
                                                  const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
                                                  const std::shared_ptr<CosmosLikeBlockchainObserver> &observer,
                                                  const std::shared_ptr<CosmosLikeAccountSynchronizer> &synchronizer,
                                                  const std::shared_ptr<CosmosLikeKeychain> &keychain);

                                std::shared_ptr<api::CosmosLikeAccount> asCosmosLikeAccount();

                                void inflateOperation(CosmosLikeOperation &out,
                                                      const std::shared_ptr<const AbstractWallet> &wallet,
                                                      const cosmos::Transaction &tx,
                                                      const cosmos::Message &msg);

                                int putTransaction(soci::session &sql, const cosmos::Transaction &transaction);

                                bool putBlock(soci::session &sql, const api::Block &block);

                                std::shared_ptr<CosmosLikeKeychain> getKeychain() const;

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

                                void broadcastRawTransaction(const std::string &transaction, const std::shared_ptr<api::StringCallback> &callback) override;

                                void broadcastTransaction(const std::shared_ptr<api::CosmosLikeTransaction> &transaction, const std::shared_ptr<api::StringCallback>&callback) override;

                                std::shared_ptr<api::CosmosLikeTransactionBuilder> buildTransaction() override;
                                std::shared_ptr<api::CosmosLikeTransactionBuilder> buildTransaction(const std::string &senderAddress);

                                std::shared_ptr<api::OperationQuery> queryOperations() override;
                                void getEstimatedGasLimit(const std::shared_ptr<api::CosmosLikeTransaction> &transaction, const std::shared_ptr<api::BigIntCallback> &callback) override;
                        private:
                                std::shared_ptr<CosmosLikeAccount> getSelf();

                                std::shared_ptr<CosmosLikeKeychain> _keychain;
                                std::string _accountAddress;
                                std::shared_ptr<CosmosLikeBlockchainExplorer> _explorer;
                                std::shared_ptr<CosmosLikeAccountSynchronizer> _synchronizer;
                                std::shared_ptr<CosmosLikeBlockchainObserver> _observer;
                                uint64_t _currentBlockHeight;
                                std::shared_ptr<api::EventBus> _currentSyncEventBus;
                                std::mutex _synchronizationLock;
                };
        }
}
#endif //LEDGER_CORE_COSMOSLIKEACCOUNT_H
