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
#include "explorers/BitcoinLikeBlockchainExplorer.hpp"

#include <api/Amount.hpp>
#include <api/AmountCallback.hpp>
#include <api/BigIntListCallback.hpp>
#include <api/BitcoinLikeAccount.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BitcoinLikePickingStrategy.hpp>
#include <api/BitcoinLikePreparedTransaction.hpp>
#include <api/BitcoinLikeTransactionRequest.hpp>
#include <api/OperationListCallback.hpp>
#include <preferences/Preferences.hpp>
#include <soci.h>
#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <wallet/bitcoin/synchronizers/BitcoinLikeAccountSynchronizer.hpp>
#include <wallet/bitcoin/types.h>
#include <wallet/common/AbstractAccount.hpp>

namespace ledger {
    namespace core {
        class Operation;
        class BitcoinLikeUtxoPicker;

        class BitcoinLikeAccount : public api::BitcoinLikeAccount, public AbstractAccount {
          public:
            BitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                               int32_t index,
                               const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer,
                               const std::shared_ptr<BitcoinLikeAccountSynchronizer> &synchronizer,
                               const std::shared_ptr<BitcoinLikeKeychain> &keychain);

            std::shared_ptr<api::BitcoinLikeAccount> asBitcoinLikeAccount() override;

            /**
             * Interpret operations from transactions and fill a output vector
             * @param transaction The transaction to interpret
             * @param out A vector which is filled by this function
             * @param needExtendKeychain Whether we need to extend the keychain during the transaction interpretation
             */
            void interpretTransaction(const BitcoinLikeBlockchainExplorerTransaction &transaction, std::vector<Operation> &out, bool needExtendKeychain = false);

            Try<int> bulkInsert(const std::vector<Operation> &out);

            /**
             *
             * @param block
             * @return true if the block wasn't already known.
             */
            bool putBlock(soci::session &sql, const BitcoinLikeBlockchainExplorer::Block &block);

            std::shared_ptr<BitcoinLikeKeychain> getKeychain() const;

            /***
             * REVIEW
             */

            bool isSynchronizing() override;

            FuturePtr<ledger::core::Amount> getBalance() override;

            Future<std::vector<std::shared_ptr<api::Amount>>> getBalanceHistory(const std::string &start,
                                                                                const std::string &end,
                                                                                api::TimePeriod precision) override;

            FuturePtr<BitcoinLikeBlockchainExplorerTransaction> getTransaction(const std::string &hash);

            std::shared_ptr<api::EventBus> synchronize() override;

            void broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                         const std::shared_ptr<api::StringCallback> &callback) override;

            void broadcastTransaction(const std::shared_ptr<api::BitcoinLikeTransaction> &transaction,
                                      const std::shared_ptr<api::StringCallback> &callback) override;

            void broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                         const std::shared_ptr<api::StringCallback> &callback,
                                         const std::string &correlationId);

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> buildTransaction(bool partial) override;

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            FuturePtr<ledger::core::Amount> getMaxSpendable(api::BitcoinLikePickingStrategy strategy, optional<int32_t> maxUtxos);

            void getMaxSpendable(api::BitcoinLikePickingStrategy strategy, optional<int32_t> maxUtxos, const std::shared_ptr<api::AmountCallback> &callback) override;

            void getUTXO(int32_t from, int32_t to, const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) override;
            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> getUTXO();
            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> getUTXO(int32_t from, int32_t to);
            void getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) override;
            Future<int32_t> getUTXOCount();

            Future<AddressList> getFreshPublicAddresses() override;

            Future<std::string> broadcastTransaction(const std::vector<uint8_t> &transaction);

            std::string getRestoreKey() override;

            const std::shared_ptr<BitcoinLikeBlockchainExplorer> &getExplorer() const;

            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point &date) override;

            void getFees(const std::shared_ptr<api::BigIntListCallback> &callback) override;

            Future<AbstractAccount::AddressList> getAddresses(int64_t from, int64_t to);
            void getAddresses(int64_t from, int64_t to, const std::shared_ptr<api::AddressListCallback> &callback) override;

            AbstractAccount::AddressList getAllAddresses() override;

            std::shared_ptr<api::Keychain> getAccountKeychain() override;

          protected:
            bool checkIfWalletIsEmpty();

          private:
            std::shared_ptr<BitcoinLikeAccount> getSelf();
            inline void inflateOperation(Operation &out,
                                         const BitcoinLikeBlockchainExplorerTransaction &tx);
            inline void computeOperationTrust(Operation &operation,
                                              const BitcoinLikeBlockchainExplorerTransaction &tx);
            std::vector<std::shared_ptr<api::Address>> fromBitcoinAddressesToAddresses(const std::vector<std::shared_ptr<BitcoinLikeAddress>> &addresses);

            std::shared_ptr<BitcoinLikeKeychain> _keychain;
            std::shared_ptr<BitcoinLikeBlockchainExplorer> _explorer;
            std::shared_ptr<BitcoinLikeAccountSynchronizer> _synchronizer;
            std::shared_ptr<BitcoinLikeUtxoPicker> _picker;
            std::shared_ptr<api::EventBus> _currentSyncEventBus;
            std::mutex _synchronizationLock;
            uint64_t _currentBlockHeight;
        };
    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_BITCOINLIKEACCOUNT_HPP
