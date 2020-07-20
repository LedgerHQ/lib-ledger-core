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


#pragma once

#include <algorand/AlgorandAddress.hpp>
#include <algorand/AlgorandAccountSynchronizer.hpp>
#include <algorand/AlgorandBlockchainExplorer.hpp>
#include <algorand/AlgorandBlockchainObserver.hpp>
#include <algorand/model/AlgorandAccount.hpp>
#include <algorand/model/transactions/AlgorandTransaction.hpp>
#include <algorand/operations/AlgorandOperation.hpp>
#include <algorand/operations/AlgorandOperationQuery.hpp>

#include <algorand/api/AlgorandAccount.hpp>
#include <algorand/api/AlgorandAssetAmountCallback.hpp>
#include <algorand/api/AlgorandAssetAmountListCallback.hpp>
#include <algorand/api/AlgorandAssetAmountListCallback.hpp>
#include <algorand/api/AlgorandAssetParamsCallback.hpp>
#include <algorand/api/AlgorandAssetParamsListCallback.hpp>
#include <algorand/api/AlgorandTransaction.hpp>
#include <algorand/api/AlgorandTransactionCallback.hpp>
#include <algorand/api/AmountCallback.hpp>
#include <algorand/api/BoolCallback.hpp>
#include <algorand/api/StringCallback.hpp>

#include <core/wallet/AbstractAccount.hpp>

#include <mutex>

namespace ledger {
namespace core {
namespace algorand {

    class Account : public api::AlgorandAccount, public AbstractAccount {

    public:

        static constexpr int FLAG_TRANSACTION_IGNORED = 0x00;

        Account(const std::shared_ptr<AbstractWallet>& wallet,
                int32_t index,
                const api::Currency& currency,
                const std::string& address,
                std::shared_ptr<BlockchainExplorer> explorer,
                std::shared_ptr<BlockchainObserver> observer,
                std::shared_ptr<AccountSynchronizer> synchronizer);


        bool putBlock(soci::session& sql, const api::Block& block);

        int putTransaction(soci::session& sql, const model::Transaction& transaction);

        const Address& getAddress() const;

        // From api::AlgorandAccount
        void getSpendableBalance(
                api::AlgorandOperationType operationType,
                const std::shared_ptr<api::AmountCallback>& callback) override;

        void getAsset(
                const std::string& assetId,
                const std::shared_ptr<api::AlgorandAssetParamsCallback>& callback) override;

        void hasAsset(
                const std::string& address,
                const std::string& assetId,
                const std::shared_ptr<api::BoolCallback>& callback) override;

        void getAssetBalance(
                const std::string& assetId,
                const std::shared_ptr<api::AlgorandAssetAmountCallback>& callback) override;

        Future<std::vector<api::AlgorandAssetAmount>> getAssetBalanceHistory(
                const std::string& assetId,
                const std::string& start,
                const std::string& end,
                api::TimePeriod period);

        void getAssetBalanceHistory(
                const std::string& assetId,
                const std::string& start,
                const std::string& end,
                api::TimePeriod period,
                const std::shared_ptr<api::AlgorandAssetAmountListCallback>& callback) override;

        void getAssetsBalances(
                const std::shared_ptr<api::AlgorandAssetAmountListCallback>& callback) override;

        void getCreatedAssets(
                const std::shared_ptr<api::AlgorandAssetParamsListCallback>& callback) override;


        void getPendingRewards(
                const std::shared_ptr<api::AmountCallback>& callback) override;

        void getTotalRewards(
                const std::shared_ptr<api::AmountCallback>& callback) override;

        void getFeeEstimate(
                const std::shared_ptr<api::AlgorandTransaction>& transaction,
                const std::shared_ptr<api::AmountCallback>& callback) override;

        void broadcastRawTransaction(
                const std::vector<uint8_t>& transaction,
                const std::shared_ptr<api::StringCallback>& callback) override;

        void broadcastTransaction(
                const std::shared_ptr<api::AlgorandTransaction>& transaction,
                const std::shared_ptr<api::StringCallback>& callback) override;

        void createTransaction(
                const std::shared_ptr<api::AlgorandTransactionCallback>& callback) override;

        // From api::Account
        std::shared_ptr<api::OperationQuery> queryOperations() override;

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
        std::shared_ptr<Account> getSelf();

        Future<model::Account> getAccountInformation() const;

    private:
        Address _address;
        std::shared_ptr<BlockchainExplorer> _explorer;
        std::shared_ptr<BlockchainObserver> _observer;
        std::shared_ptr<AccountSynchronizer> _synchronizer;
        std::shared_ptr<api::EventBus> _currentSyncEventBus;
        std::mutex _synchronizationLock;
    };

} // namespace algorand
} // namespace core
} // namespace ledger

