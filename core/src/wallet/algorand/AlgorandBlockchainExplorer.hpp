/*
 * AlgorandBlockchainExplorer
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

#ifndef LEDGER_CORE_ALGORANDBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_ALGORANDBLOCKCHAINEXPLORER_H

#include <algorand/api/AlgorandTransaction.hpp>
#include <algorand/api/AlgorandNetworkParameters.hpp>
#include <algorand/model/transactions/AlgorandTransaction.hpp>
#include <algorand/model/transactions/AlgorandTransactionParams.hpp>
#include <algorand/model/AlgorandAccount.hpp>

#include <core/async/DedicatedContext.hpp>
#include <core/utils/ConfigurationMatchable.hpp>

#include <core/api/Block.hpp>
#include <core/net/HttpClient.hpp>

namespace ledger {
namespace core {
namespace algorand {
namespace constants {

    static const std::string purestakeTokenHeader = "x-api-key";

    // Explorer endpoints
    static const std::string purestakeBlockEndpoint = "/block/{}";
    static const std::string purestakeAccountEndpoint = "/account/{}";
    static const std::string purestakeAccountTransactionsEndpoint = "/account/{}/transactions";
    static const std::string purestakeTransactionEndpoint = "/transaction/{}";
    static const std::string purestakeTransactionsEndpoint = "/transactions";
    static const std::string purestakeTransactionsParamsEndpoint = "/transactions/params";

} // namespace constants

    class BlockchainExplorer : public ConfigurationMatchable, public DedicatedContext {

    public:

        BlockchainExplorer(const std::shared_ptr<api::ExecutionContext> &context,
                           const std::shared_ptr<HttpClient> &http,
                           const api::AlgorandNetworkParameters &parameters,
                           const std::shared_ptr<api::DynamicObject> &configuration);

        // CurrentBlock querier
        FuturePtr<api::Block> getCurrentBlock() const;

        // Block querier
        FuturePtr<api::Block> getBlock(uint64_t & blockHeight) const;

        // Account querier
        FuturePtr<model::Account> getAccount(const std::string & address) const;

        // Single transaction querier (found by hash)
        FuturePtr<model::Transaction> getTransactionById(const std::string & txId) const;

        // Get all transactions relevant to an address
        // Concatenates multiple API calls for all relevant transaction types
        FuturePtr<model::TransactionsBulk> getTransactionsForAddress(const std::string &address, const uint64_t & fromBlockHeight = 0) const;

        // Get suggested fee for a transaction
        Future<uint64_t> getSuggestedFee(const std::shared_ptr<api::AlgorandTransaction> & transaction) const;

        FuturePtr<model::TransactionParams> getTransactionParams() const;

        Future<std::string> pushTransaction(const std::vector<uint8_t> & transaction);

    private:

        std::shared_ptr<HttpClient> _http;
        api::AlgorandNetworkParameters _parameters;

    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDBLOCKCHAINEXPLORER_H
