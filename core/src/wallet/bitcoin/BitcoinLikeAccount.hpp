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
#ifndef LEDGER_CORE_BITCOINLIKEACCOUNT_HPP
#define LEDGER_CORE_BITCOINLIKEACCOUNT_HPP

#include "BitcoinLikeWallet.hpp"
#include <api/BitcoinLikeAccount.hpp>
#include "explorers/BitcoinLikeBlockchainExplorer.hpp"
#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <soci.h>
#include <preferences/Preferences.hpp>
#include <wallet/common/AbstractAccount.hpp>
#include <api/Amount.hpp>
#include <api/AmountCallback.hpp>
#include <api/OperationListCallback.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BitcoinLikePickingStrategy.hpp>
#include <api/BitcoinLikeTransactionRequest.hpp>
#include <api/BitcoinLikePreparedTransaction.hpp>
#include <wallet/bitcoin/types.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeUtxoPicker.h>

#include <wallet/bitcoin/synchronizers/BitcoinLikeAccountSynchronizer.hpp>

namespace ledger {
    namespace core {
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
                               const std::shared_ptr<BitcoinLikeBlockchainExplorer>& explorer,
                               const std::shared_ptr<BitcoinLikeBlockchainObserver>& observer,
                               const std::shared_ptr<BitcoinLikeAccountSynchronizer>& synchronizer,
                               const std::shared_ptr<BitcoinLikeKeychain>& keychain
            );

            std::shared_ptr<api::BitcoinLikeAccount> asBitcoinLikeAccount() override;

            /**
             *
             * @param transaction
             * @return A flag indicating if the transaction was ignored, inserted
             */
            int putTransaction(soci::session& sql, const BitcoinLikeBlockchainExplorerTransaction& transaction);
            /**
             *
             * @param block
             * @return true if the block wasn't already known.
             */
            bool putBlock(soci::session& sql, const BitcoinLikeBlockchainExplorer::Block& block);

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

            FuturePtr<BitcoinLikeBlockchainExplorerTransaction> getTransaction(const std::string& hash);

            std::shared_ptr<api::EventBus> synchronize() override;

            void broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                         const std::shared_ptr<api::StringCallback> &callback) override;

            void broadcastTransaction(const std::shared_ptr<api::BitcoinLikeTransaction> &transaction,
                                      const std::shared_ptr<api::StringCallback> &callback) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> buildTransaction(std::experimental::optional<bool> partial) override;

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            void getUTXO(int32_t from, int32_t to,
                         const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) override;
            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> getUTXO();
            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> getUTXO(int32_t from, int32_t to);
            void getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) override;
            Future<int32_t> getUTXOCount();

            Future<AddressList> getFreshPublicAddresses() override;

            Future<std::string> broadcastTransaction(const std::vector<uint8_t>& transaction);

            std::string getRestoreKey() override;

            const std::shared_ptr<BitcoinLikeBlockchainExplorer>& getExplorer() const;

            stlab::future<void> eraseDataSince(const std::chrono::system_clock::time_point & date) override ;

        protected:
            bool checkIfWalletIsEmpty();

        private:
            std::shared_ptr<BitcoinLikeAccount> getSelf();
            inline void inflateOperation(Operation& out,
                                         const std::shared_ptr<const AbstractWallet>& wallet,
                                         const BitcoinLikeBlockchainExplorerTransaction& tx);
            inline void computeOperationTrust(Operation& operation,
                                              const std::shared_ptr<const AbstractWallet>& wallet,
                                              const BitcoinLikeBlockchainExplorerTransaction& tx);

        private:
            std::shared_ptr<BitcoinLikeKeychain> _keychain;
            std::shared_ptr<BitcoinLikeBlockchainExplorer> _explorer;
            std::shared_ptr<BitcoinLikeAccountSynchronizer> _synchronizer;
            std::shared_ptr<BitcoinLikeBlockchainObserver> _observer;
            std::shared_ptr<BitcoinLikeUtxoPicker> _picker;
            std::shared_ptr<api::EventBus> _currentSyncEventBus;
            std::mutex _synchronizationLock;
            uint64_t _currentBlockHeight;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEACCOUNT_HPP
