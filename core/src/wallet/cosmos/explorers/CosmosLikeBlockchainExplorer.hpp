/*
 *
 * CosmosLikeBlockchainExplorer
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


#ifndef LEDGER_CORE_COSMOSLIKEBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_COSMOSLIKEBLOCKCHAINEXPLORER_H


#include <string>

#include <wallet/common/Block.h>
#include <api/DynamicObject.hpp>
#include <api/ExecutionContext.hpp>
#include <async/DedicatedContext.hpp>
#include <collections/DynamicObject.hpp>
#include <wallet/common/explorers/AbstractBlockchainExplorer.h>
#include <math/BigInt.h>
#include <net/HttpClient.hpp>
#include <utils/ConfigurationMatchable.h>
#include <utils/Option.hpp>

#include <wallet/cosmos/keychains/CosmosLikeKeychain.hpp>
#include <api/CosmosLikeNetworkParameters.hpp>
#include <api/CosmosLikeTransaction.hpp>
#include <wallet/cosmos/cosmos.hpp>

namespace ledger {
    namespace core {

        class CosmosLikeBlockchainExplorer : public ConfigurationMatchable {
        public:
            using TransactionFilter = std::string;

            CosmosLikeBlockchainExplorer(
                const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
                const std::vector<std::string> &matchableKeys);


            virtual FuturePtr<std::vector<cosmos::Delegation>> getDelegations(const std::string& delegatorAddr) const = 0;
            virtual FuturePtr<std::vector<cosmos::Reward>> getPendingRewards(const std::string& delegatorAddr) const = 0;

            // Everything below is c/p from AbstractBlockchainExplorer, in sole purpose
            // of being able to use fromBlockHeight instead of fromBlockHash in getTransactions (see below)
            // TODO Maybe some code cleanup and/or factorization

            virtual Future<void *> startSession() = 0;
            virtual Future<Unit> killSession(void *session) = 0;
            virtual Future<Bytes> getRawTransaction(const String& transactionHash) = 0;
            virtual Future<String> pushTransaction(const std::vector<uint8_t>& transaction) = 0;
            virtual Future<int64_t> getTimestamp() const = 0;
            virtual FuturePtr<cosmos::Block> getCurrentBlock() const = 0;
            virtual FuturePtr<cosmos::Block> getCurrentBlock() = 0;
            virtual FuturePtr<cosmos::Block> getBlock(uint64_t& blockHeight) = 0;
            virtual FuturePtr<cosmos::Account> getAccount(const std::string& account) const  = 0;
            virtual const std::vector<TransactionFilter>& getTransactionFilters() = 0;

            // Pending statuses
            virtual Future<cosmos::UnbondingList> getUnbondingsByDelegator(const std::string& delegatorAddress) const = 0;
            virtual Future<cosmos::RedelegationList> getRedelegationsByDelegator(const std::string& delegatorAddress) const = 0;

            // Balances
            /// Get Total Balance
            virtual FuturePtr<BigInt> getTotalBalance(const std::string& account) const = 0;
            /// Get Total Balance except pending rewards
            virtual FuturePtr<BigInt> getTotalBalanceWithoutPendingRewards(const std::string& account) const = 0;
            /// Get total balance in delegation
            virtual FuturePtr<BigInt> getDelegatedBalance(const std::string& account) const = 0;
            /// Get total pending rewards
            virtual FuturePtr<BigInt> getPendingRewardsBalance(const std::string& account) const = 0;
            /// Get total unbonding balance
            virtual FuturePtr<BigInt> getUnbondingBalance(const std::string& account) const = 0;
            /// Get total available (spendable) balance
            virtual FuturePtr<BigInt> getSpendableBalance(const std::string& account) const = 0;

            virtual FuturePtr<cosmos::Transaction> getTransactionByHash(const String& transactionHash) const = 0;
            virtual Future<std::shared_ptr<cosmos::Transaction>> getTransactionByHash(const std::string& hash) = 0;
            virtual FuturePtr<cosmos::TransactionsBulk> getTransactions(const std::vector<std::string>& addresses,
                                                                        uint32_t fromBlockHeight = 0,
                                                                        Option<void*> session = Option<void *>()) = 0;

            // Validators
            virtual Future<cosmos::ValidatorList> getActiveValidatorSet() const = 0;
            virtual Future<cosmos::Validator> getValidatorInfo(const std::string& valOperAddress) const = 0;

            virtual FuturePtr<BigInt> getEstimatedGasLimit(const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
                                                           double gasAdjustment = 1.0) const = 0;
        };
    }
}


#endif //LEDGER_CORE_COSMOSLIKEBLOCKCHAINEXPLORER_H
