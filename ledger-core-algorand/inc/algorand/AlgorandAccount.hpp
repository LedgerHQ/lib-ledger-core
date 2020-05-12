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

#include <algorand/AlgorandAddress.hpp>

#include <algorand/api/AlgorandAccount.hpp>
#include <algorand/api/AlgorandTransaction.hpp>
#include <algorand/api/AlgorandAssetParamsListCallback.hpp>
#include <algorand/api/AlgorandAssetAmountCallback.hpp>
#include <algorand/api/AmountCallback.hpp>

#include <core/wallet/AbstractAccount.hpp>

namespace ledger {
namespace core {
namespace algorand {

    class Account : public api::AlgorandAccount, public AbstractAccount {

    // TODO implementation; this is currently just a mock up

    public:

        Account(const std::shared_ptr<AbstractWallet> &wallet,
                int32_t index,
                const api::Currency& currency,
                const std::vector<uint8_t> & pubKey) :
            AbstractAccount(wallet->getServices(), wallet, index),
            _address(currency, pubKey)
            {}


        // From api::algorandAccount
        void getAssets(const std::shared_ptr<api::AlgorandAssetParamsListCallback> & callback) override {}

        void getCreatedAssets(const std::shared_ptr<api::AlgorandAssetParamsListCallback> & callback) override {}

        void getAssetBalance(const std::string & assetId, const std::shared_ptr<api::AlgorandAssetAmountCallback> & callback) override {}

        void getPendingReward(const std::shared_ptr<api::AmountCallback> & callback) override {}

        void getFeeEstimate(const std::shared_ptr<api::AlgorandTransaction> & transaction, const std::shared_ptr<api::AmountCallback> & callback) override {}

        void broadcastTransaction(const std::shared_ptr<api::AlgorandTransaction> & transaction) override {}


        // From api::Account
        std::shared_ptr<api::OperationQuery> queryOperations() override { return std::shared_ptr<api::OperationQuery>(nullptr); }

        bool isSynchronizing() override { return false; }

        std::shared_ptr<api::EventBus> synchronize() override { return std::shared_ptr<api::EventBus>(nullptr); }

        void startBlockchainObservation() override {}

        void stopBlockchainObservation() override {}

        bool isObservingBlockchain() override { return false; }

        std::string getRestoreKey() override { return ""; }


        // From AbstractAccount
        FuturePtr<Amount> getBalance() override
            { return FuturePtr<Amount>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever")); }

        Future<std::vector<std::shared_ptr<api::Amount>>> getBalanceHistory(const std::string & start,
                                                                            const std::string & end,
                                                                            api::TimePeriod precision) override
            { return Future<std::vector<std::shared_ptr<api::Amount>>>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever")); }

        Future<AddressList> getFreshPublicAddresses() override
            { return Future<AddressList>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever")); }

        Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date) override
            { return Future<api::ErrorCode>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever")); }

    private:

        algorand::Address _address;

    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDACCOUNT_H
