/*
 *
 * BitcoinLikeAccount
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/04/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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

#include "BitcoinLikeWallet.hpp"

#include <api/Amount.hpp>
#include <api/AmountCallback.hpp>
#include <api/BitcoinLikeAccount.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BitcoinLikePickingStrategy.hpp>
#include <api/BitcoinLikeTransactionRequest.hpp>
#include <api/BitcoinLikePreparedTransaction.hpp>
#include <api/OperationListCallback.hpp>
#include <preferences/Preferences.hpp>
#include <wallet/AccountSynchronizer.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/TransactionBroadcaster.hpp>
#include <wallet/bitcoin/types.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeUtxoPicker.h>
#include <wallet/bitcoin/observers/BitcoinLikeBlockchainObserver.hpp>
#include <wallet/common/AbstractAccount.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class Operation;
            class BitcoinLikeUtxoPicker;
            class BitcoinLikeAccount : public api::BitcoinLikeAccount, public AbstractAccount {
            public:
                static const int FLAG_NEW_TRANSACTION = 0x01;
                static const int FLAG_TRANSACTION_UPDATED = 0x01 << 1;
                static const int FLAG_TRANSACTION_IGNORED = 0x00;
                static const int FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS = 0x01 << 2;
                static const int FLAG_TRANSACTION_ON_USED_ADDRESS = 0x01 << 3;
                static const int FLAG_TRANSACTION_CREATED_SENDING_OPERATION = 0x01 << 4;
                static const int FLAG_TRANSACTION_CREATED_RECEPTION_OPERATION = 0x01 << 5;

                BitcoinLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                    int32_t index,
                    const std::shared_ptr<TransactionBroadcaster<BitcoinLikeNetwork>>& broadcaster,
                    const std::shared_ptr<BitcoinLikeBlockchainObserver>& observer,
                    const std::shared_ptr<core::AccountSynchronizer>& synchronizer,
                    const std::shared_ptr<BitcoinLikeKeychain>& keychain);

                std::shared_ptr<api::BitcoinLikeAccount> asBitcoinLikeAccount() override;

                std::shared_ptr<BitcoinLikeKeychain> getKeychain() const;

                void startBlockchainObservation() override;
                void stopBlockchainObservation() override;
                bool isObservingBlockchain() override;

                /***
                 * REVIEW
                 */

                bool isSynchronizing() override;

                FuturePtr<ledger::core::Amount> getBalance() override;

                Future<std::vector<std::shared_ptr<api::Amount>>> getBalanceHistory(const std::string & start,
                    const std::string & end,
                    api::TimePeriod precision) override;

                FuturePtr<BitcoinLikeNetwork::Transaction> getTransaction(const std::string& hash);

                std::shared_ptr<api::EventBus> synchronize() override;

                void broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                    const std::shared_ptr<api::StringCallback> &callback) override;

                void broadcastTransaction(const std::shared_ptr<api::BitcoinLikeTransaction> &transaction,
                    const std::shared_ptr<api::StringCallback> &callback) override;

                std::shared_ptr<api::BitcoinLikeTransactionBuilder> buildTransaction() override;

                std::shared_ptr<api::OperationQuery> queryOperations() override;

                void getUTXO(int32_t from, int32_t to,
                    const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) override;
                Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> getUTXO();
                Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> getUTXO(int32_t from, int32_t to);
                void getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) override;
                Future<int32_t> getUTXOCount();

                Future<AddressList> getFreshPublicAddresses() override;

                std::string getRestoreKey() override;

                Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date) override;

            protected:
                bool checkIfWalletIsEmpty();

            private:
                std::shared_ptr<BitcoinLikeAccount> getSelf();
                inline void inflateOperation(core::Operation& out,
                    const std::shared_ptr<const AbstractWallet>& wallet,
                    const BitcoinLikeNetwork::Transaction& tx);
                inline void computeOperationTrust(core::Operation& operation,
                    const std::shared_ptr<const AbstractWallet>& wallet,
                    const BitcoinLikeNetwork::Transaction& tx);

            private:
                std::shared_ptr<TransactionBroadcaster<BitcoinLikeNetwork>> _broadcaster;
                std::shared_ptr<BitcoinLikeBlockchainObserver> _observer;
                std::shared_ptr<BitcoinLikeKeychain> _keychain;
                std::shared_ptr<Preferences> _internalPreferences;
                std::shared_ptr<Preferences> _externalPreferences;
                std::shared_ptr<core::AccountSynchronizer> _synchronizer;
                std::shared_ptr<BitcoinLikeUtxoPicker> _picker;
                std::shared_ptr<api::EventBus> _currentSyncEventBus;
                std::mutex _synchronizationLock;
                uint64_t _currentBlockHeight;
            };
        }
    }
}
