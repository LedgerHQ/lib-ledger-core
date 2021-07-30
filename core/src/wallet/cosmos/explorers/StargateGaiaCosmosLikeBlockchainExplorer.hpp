/*
 *
 * StargateGaiaCosmosLikeBlockchainExplorer.hpp
 * ledger-core
 *
 * Created by Gerry Agbobada on 12/11/2020.
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

#include <boost/utility/string_view.hpp>

#include <async/DedicatedContext.hpp>
#include <net/HttpClient.hpp>
#include <wallet/common/Block.h>
#include <wallet/cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>

namespace ledger {
namespace core {

class StargateGaiaCosmosLikeBlockchainExplorer :
    public CosmosLikeBlockchainExplorer,
    public DedicatedContext {
   public:
    StargateGaiaCosmosLikeBlockchainExplorer(
        const std::shared_ptr<api::ExecutionContext> &context,
        const std::shared_ptr<HttpClient> &http,
        const api::Currency currency,
        const std::shared_ptr<api::DynamicObject> &configuration);

    // Build a URL encoded filter for gaia REST event-like filters
    // eventType.attributeKey=value
    static TransactionFilter filterWithAttribute(
        const char eventType[], const char attributeKey[], const std::string &value);

    // Concatenate multiple URL encoded filters
    static TransactionFilter fuseFilters(std::initializer_list<boost::string_view> filters);

    // TransactionFilters getter
    const std::vector<TransactionFilter> &getTransactionFilters() override;

    // Block querier
    FuturePtr<cosmos::Block> getBlock(uint64_t &blockHeight) const override;

    // Account querier
    FuturePtr<ledger::core::cosmos::Account> getAccount(const std::string &account) const override;

    // CurrentBlock querier
    FuturePtr<cosmos::Block> getCurrentBlock() override;

    // Helper function to get transactions following a given filter.
    FuturePtr<cosmos::TransactionsBulk> getTransactions(
        const TransactionFilter &filter, int page, int limit) const;

    // Single transaction querier (found by hash)
    FuturePtr<cosmos::Transaction> getTransactionByHash(const std::string &hash) override;

    Future<void *> startSession() override;
    Future<Unit> killSession(void *session) override;
    FuturePtr<cosmos::TransactionsBulk> getTransactions(
        const std::vector<std::string> &addresses,
        uint32_t fromBlockHeight = 0,
        Option<void *> session = Option<void *>()) override;
    FuturePtr<ledger::core::Block> getCurrentBlock() const override;
    [[noreturn]] Future<Bytes> getRawTransaction(const String &transactionHash) override;
    FuturePtr<cosmos::Transaction> getTransactionByHash(
        const String &transactionHash) const override;
    Future<String> pushTransaction(const std::vector<uint8_t> &transaction) override;
    Future<int64_t> getTimestamp() const override;

    // Pending statuses
    Future<cosmos::UnbondingList> getUnbondingsByDelegator(
        const std::string &delegatorAddress) const override;
    Future<cosmos::RedelegationList> getRedelegationsByDelegator(
        const std::string &delegatorAddress) const override;

    // Balances
    /// Get Total Balance
    FuturePtr<BigInt> getTotalBalance(const std::string &account) const override;
    /// Get Total Balance except pending rewards
    FuturePtr<BigInt> getTotalBalanceWithoutPendingRewards(
        const std::string &account) const override;
    /// Get total balance in delegation
    FuturePtr<BigInt> getDelegatedBalance(const std::string &account) const override;
    /// Get total pending rewards
    FuturePtr<BigInt> getPendingRewardsBalance(const std::string &account) const override;
    /// Get total unbonding balance
    FuturePtr<BigInt> getUnbondingBalance(const std::string &account) const override;
    /// Get total available (spendable) balance
    FuturePtr<BigInt> getSpendableBalance(const std::string &account) const override;

    // Validators
    Future<cosmos::ValidatorList> getActiveValidatorSet() const override;
    Future<cosmos::Validator> getValidatorInfo(const std::string &valOperAddress) const override;

    FuturePtr<std::vector<cosmos::Delegation>> getDelegations(
        const std::string &delegatorAddr) const override;
    FuturePtr<std::vector<cosmos::Reward>> getPendingRewards(
        const std::string &delegatorAddr) const override;
    /// Get the estimated gas needed to broadcast the transaction
    FuturePtr<BigInt> getEstimatedGasLimit(
        const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
        const std::string gasAdjustment = "1.0") const override;

   private:

    /// Parse a transaction and add post-treatment / sanitization.
    /// The sanitization of output includes :
    /// - Adding the FESS as an extra message
    /// - Query more information about the block included in the transaction
    /// \param[in] node The (rapidjson) node with the transaction data to use
    /// \param[out] transaction The Cosmos transaction to fill
    template <typename T>
    void parseTransactionWithPosttreatment(const T &node, cosmos::Transaction &transaction) const;

    /// Inflate a transaction with all its block data (block hash and timestamp)
    /// The hash information is necessary to compute a block_uid and therefore not having
    /// it will prevent the OperationQuery from fetching the correct block to compute the
    /// number of confirmations for a given transaction.
    /// \param[in] The transaction to fill
    /// \return a FuturePtr to the filled Transaction
    FuturePtr<cosmos::Transaction> inflateTransactionWithBlockData(const cosmos::Transaction& inputTx) const;

    // Get all transactions relevant to an address
    // Concatenates multiple API calls for all relevant transaction types
    FuturePtr<cosmos::TransactionsBulk> getTransactionsForAddress(
        const std::string &address, uint32_t fromBlockHeight = 0) const;

    // Get all transactions relevant to a list of addresses
    // Concatenates multiple API calls for all relevant transaction types
    FuturePtr<cosmos::TransactionsBulk> getTransactionsForAddresses(
        const std::vector<std::string> &addresses, uint32_t fromBlockHeight = 0) const;

    Future<BigInt> genericPostRequestForSimulation(
        const std::string &endpoint, const std::string &transaction) const;

    Future<BigInt> getEstimatedGasLimit(
        const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
        const std::shared_ptr<api::CosmosLikeMessage> &message,
        const std::string gasAdjustment = "1.0") const;
    Future<BigInt> getEstimatedGasLimitForTransfer(
        const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
        const std::shared_ptr<api::CosmosLikeMessage> &message,
        const std::string gasAdjustment = "1.0") const;
    Future<BigInt> getEstimatedGasLimitForRewards(
        const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
        const std::shared_ptr<api::CosmosLikeMessage> &message,
        const std::string gasAdjustment = "1.0") const;
    Future<BigInt> getEstimatedGasLimitForDelegations(
        const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
        const std::shared_ptr<api::CosmosLikeMessage> &message,
        const std::string gasAdjustment = "1.0") const;
    Future<BigInt> getEstimatedGasLimitForUnbounding(
        const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
        const std::shared_ptr<api::CosmosLikeMessage> &message,
        const std::string gasAdjustment = "1.0") const;
    Future<BigInt> getEstimatedGasLimitForRedelegations(
        const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
        const std::shared_ptr<api::CosmosLikeMessage> &message,
        const std::string gasAdjustment = "1.0") const;

   private:
    std::shared_ptr<HttpClient> _http;
    api::Currency _currency;
    /// GRPC namespace of the endpoints to use
    std::string _ns{"cosmos"};
    /// GRPC version of the endpoints to use
    std::string _version{"v1beta1"};
    /// Add a fees message to a transaction for internal purpose
    template <typename T>
    void addMsgFeesTo(cosmos::Transaction &transaction, const T &node) const;
};

}  // namespace core
}  // namespace ledger
