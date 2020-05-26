/*
 * AlgorandAccount
 *
 * Created by Hakim Aammar on 20/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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


#ifndef LEDGER_CORE_ALGORANDACCOUNT_H
#define LEDGER_CORE_ALGORANDACCOUNT_H

#include "AlgorandAddress.hpp"
#include "operations/AlgorandOperation.hpp"
#include "model/transactions/AlgorandTransaction.hpp"

#include <api/AlgorandAccount.hpp>
#include <api/AlgorandTransaction.hpp>
#include <api/AlgorandAssetParamsListCallback.hpp>
#include <api/AlgorandAssetAmountCallback.hpp>
#include <api/AmountCallback.hpp>
#include <api/Keychain.hpp>

#include <wallet/common/AbstractAccount.hpp>

namespace ledger {
namespace core {
namespace algorand {

    class Account : public api::AlgorandAccount, public AbstractAccount {

    public:

        static constexpr int FLAG_TRANSACTION_IGNORED = 0x00;

        Account(const std::shared_ptr<AbstractWallet>& wallet,
                int32_t index,
                const api::Currency& currency,
                const std::vector<uint8_t>& pubKey);


        bool putBlock(soci::session& sql, const api::Block& block);

        int putTransaction(soci::session& sql, const model::Transaction& transaction);

        // From api::AlgorandAccount
        void getAssets(const std::shared_ptr<api::AlgorandAssetParamsListCallback>& callback) override;

        void getCreatedAssets(const std::shared_ptr<api::AlgorandAssetParamsListCallback>& callback) override;

        void getAssetBalance(const std::string& assetId,
                             const std::shared_ptr<api::AlgorandAssetAmountCallback>& callback) override;

        void getPendingReward(const std::shared_ptr<api::AmountCallback>& callback) override;

        void getFeeEstimate(const std::shared_ptr<api::AlgorandTransaction>& transaction,
                            const std::shared_ptr<api::AmountCallback>& callback) override;

        void broadcastTransaction(const std::shared_ptr<api::AlgorandTransaction>& transaction) override;

        // From api::Account
        std::shared_ptr<api::OperationQuery> queryOperations() override;
        std::shared_ptr<api::Keychain> getAccountKeychain() override;

        bool isSynchronizing() override;

        std::shared_ptr<api::EventBus> synchronize() override;

        void startBlockchainObservation() override;

        void stopBlockchainObservation() override;

        bool isObservingBlockchain() override;

        std::string getRestoreKey() override;


        // From AbstractAccount
        FuturePtr<Amount> getBalance() override;

        Future<std::vector<std::shared_ptr<api::Amount>>>
        getBalanceHistory(const std::string& start,
                          const std::string& end,
                          api::TimePeriod precision) override;

        Future<AddressList> getFreshPublicAddresses() override;

        Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point& date) override;

    private:

        void inflateOperation(Operation& op,
                              const std::shared_ptr<const AbstractWallet>& wallet,
                              const model::Transaction& tx);

        algorand::Address _address;

    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDACCOUNT_H
