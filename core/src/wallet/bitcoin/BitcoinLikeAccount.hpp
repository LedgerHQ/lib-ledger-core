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

namespace ledger {
    namespace core {
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

            /**
             *
             * @param transaction
             * @return A flag indicating if the transaction was ignored, inserted
             */
            int putTransaction(soci::session& sql, const BitcoinLikeBlockchainExplorer::Transaction& transaction);
            /**
             *
             * @param block
             * @return true if the block wasn't already known.
             */
            bool putBlock(soci::session& sql, const BitcoinLikeBlockchainExplorer::Block block);

            std::shared_ptr<const BitcoinLikeKeychain> getKeychain() const;


            /***
             * REVIEW
             */
            void getOperations(int32_t from, int32_t to, bool descending, bool complete,
                               const std::shared_ptr<api::OperationListCallback> &callback) override;

            void getOperationsCount(const std::shared_ptr<api::I64Callback> &callback) override;

            void getOperation(const std::string &uid, const std::shared_ptr<api::OperationCallback> &callback) override;

            void getBalance(const std::shared_ptr<api::AmountCallback> &callback) override;

            bool isSynchronizing() override;

            std::shared_ptr<api::EventBus> synchronize() override;

            void computeFees(const std::shared_ptr<api::Amount> &amount, int32_t priority,
                             const std::vector<std::string> &recipients, const std::vector<std::vector<uint8_t>> &data,
                             const std::shared_ptr<api::AmountCallback> &callback) override;

            void getUTXO(int32_t from, int32_t to,
                         const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) override;

            void getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) override;

        private:
            std::shared_ptr<BitcoinLikeKeychain> _keychain;
            std::shared_ptr<Preferences> _internalPreferences;
            std::shared_ptr<Preferences> _externalPreferences;
            std::shared_ptr<BitcoinLikeBlockchainExplorer> _explorer;
            std::shared_ptr<BitcoinLikeAccountSynchronizer> _synchronizer;
            std::shared_ptr<BitcoinLikeBlockchainObserver> _observer;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEACCOUNT_HPP
