/*
 *
 * GaiaCosmosLikeBlockchainExplorer.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/11/2019.
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

#pragma once

#include <cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>

#include <core/async/DedicatedContext.hpp>
#include <core/net/HttpClient.hpp>
#include <core/api/Block.hpp>

#include <experimental/string_view>

namespace ledger {
    namespace core {

        class GaiaCosmosLikeBlockchainExplorer :
                public CosmosLikeBlockchainExplorer,
                public DedicatedContext {
        public:
            GaiaCosmosLikeBlockchainExplorer(
                    const std::shared_ptr<api::ExecutionContext> &context,
                    const std::shared_ptr<HttpClient> &http,
                    const api::CosmosLikeNetworkParameters &parameters,
                    const std::shared_ptr<api::DynamicObject> &configuration);

            // Build a URL encoded filter for gaia REST event-like filters
            // eventType.attributeKey=value
            static TransactionFilter filterWithAttribute(
                const char eventType[], const char attributeKey[], const std::string &value);

            // Concatenate multiple URL encoded filters
            static TransactionFilter fuseFilters(std::initializer_list<std::experimental::string_view> filters);

            // TransactionFilters getter
            const std::vector<TransactionFilter> &getTransactionFilters() override;

            // Block querier
            FuturePtr<cosmos::Block> getBlock(uint64_t &blockHeight) override;

            // Account querier
            FuturePtr<ledger::core::cosmos::Account> getAccount(const std::string &account) override;

            // CurrentBlock querier
            FuturePtr<cosmos::Block> getCurrentBlock() override;

            // Get all transactions relevant to an address
            // Concatenates multiple API calls for all relevant transaction types
            Future<cosmos::TransactionList> getTransactionsForAddress(
                const std::string &address, Option<std::string> fromBlockHash) const;

            // Get all transactions relevant to a list of addresses
            // Concatenates multiple API calls for all relevant transaction types
            Future<cosmos::TransactionList> getTransactionsForAddresses(
                const std::vector<std::string> &addresses, Option<std::string> fromBlockHash) const;

            // Helper function to get transactions following a given filter.
            Future<TransactionList> getTransactions(
                const TransactionFilter &filter, int page, int limit) const override;

            // Single transaction querier (found by hash)
            FuturePtr<cosmos::Transaction>
            getTransactionByHash(const std::string &hash) override;


            Future<void *> startSession() override;
            Future<Unit> killSession(void* session) override;
            FuturePtr<TransactionsBulk> getTransactions(const std::vector<std::string>& addresses,
                                                                Option<std::string> fromBlockHash = Option<std::string>(),
                                                                Option<void*> session = Option<void *>()) override;
            FuturePtr<cosmos::Block> getCurrentBlock() const override;
            [[ noreturn ]] Future<Bytes> getRawTransaction(const String& transactionHash) override;
            FuturePtr<cosmos::Transaction> getTransactionByHash(const String& transactionHash) const override;
            Future<String> pushTransaction(const std::vector<uint8_t>& transaction) override;
            Future<int64_t> getTimestamp() const override;

        private:
            std::shared_ptr<HttpClient> _http;
            api::CosmosLikeNetworkParameters _parameters;
        };

    }
}
