/*
 *
 * StellarLikeAccount.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
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

#ifndef LEDGER_CORE_STELLARLIKEACCOUNT_HPP
#define LEDGER_CORE_STELLARLIKEACCOUNT_HPP

#include <wallet/common/AbstractAccount.hpp>
#include <api/StellarLikeAccount.hpp>
#include <wallet/stellar/keychains/StellarLikeKeychain.hpp>
#include "synchronizers/StellarLikeAccountSynchronizer.h"
#include "explorers/StellarLikeBlockchainExplorer.hpp"
#include <api/StellarLikeTransactionBuilder.hpp>
#include <api/StellarLikeFeeStats.hpp>
#include <api/StellarLikeFeeStatsCallback.hpp>


namespace ledger {
    namespace core {

        /**
         * This structure holds every runtime module that the account is using.
         * It makes the stellar account easier to build without passing a ton of
         * parameters to its constructor. Once this structure is created it can
         * be passed to the StellarLikeAccount constructor which keep an immutable
         * copy of it.
         */
        struct StellarLikeAccountParams {
            int index;
            std::shared_ptr<StellarLikeKeychain> keychain;
            std::shared_ptr<DatabaseSessionPool> database;
            std::shared_ptr<StellarLikeAccountSynchronizer> synchronizer;
            std::shared_ptr<StellarLikeBlockchainExplorer> explorer;
        };

        class StellarLikeWallet;

        class StellarLikeAccount : public api::StellarLikeAccount, public AbstractAccount {
        public:
            StellarLikeAccount(const std::shared_ptr<StellarLikeWallet>& wallet, const StellarLikeAccountParams& params);
            bool isSynchronizing() override;
            std::shared_ptr<api::EventBus> synchronize() override;
            void startBlockchainObservation() override;
            void stopBlockchainObservation() override;
            bool isObservingBlockchain() override;
            std::string getRestoreKey() override;
            FuturePtr<Amount> getBalance() override;
            Future<AddressList> getFreshPublicAddresses() override;
            Future<std::vector<std::shared_ptr<api::Amount>>>
            getBalanceHistory(const std::string &start, const std::string &end, api::TimePeriod precision) override;
            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point &date) override;

            std::shared_ptr<StellarLikeKeychain> getKeychain() const { return _params.keychain; };

            // Data insertion methods
            int putTransaction(soci::session& sql, const stellar::Transaction& tx);

            int putLedger(soci::session& sql, stellar::Ledger& ledger);
            void updateAccountInfo(soci::session& sql, stellar::Account& account);

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            void exists(const std::shared_ptr<api::BoolCallback> &callback) override;
            Future<bool> exists();
            std::shared_ptr<api::StellarLikeTransactionBuilder> buildTransaction() override;

            void broadcastRawTransaction(const std::vector<uint8_t> &tx,
                                         const std::shared_ptr<api::StringCallback> &callback) override;
            Future<std::string> broadcastRawTransaction(const std::vector<uint8_t> &tx);

            const StellarLikeAccountParams& params() const { return _params; };

            void getBaseReserve(const std::shared_ptr<api::AmountCallback> &callback) override;
            FuturePtr<Amount> getBaseReserve();

            void getFeeStats(const std::shared_ptr<api::StellarLikeFeeStatsCallback> &callback) override;
            Future<api::StellarLikeFeeStats> getFeeStats();

            void getSequence(const std::shared_ptr<api::BigIntCallback> &callback) override;
            Future<BigInt> getSequence();

        protected:
            std::shared_ptr<StellarLikeAccount> getSelf();

        private:
            std::shared_ptr<StellarLikeWallet> _wallet;
            const StellarLikeAccountParams _params;
            std::mutex _synchronizationLock;
            std::shared_ptr<api::EventBus> _currentSyncEventBus;
            uint64_t _currentLedgerHeight;

        };
    }
}

#endif //LEDGER_CORE_STELLARLIKEACCOUNT_HPP
