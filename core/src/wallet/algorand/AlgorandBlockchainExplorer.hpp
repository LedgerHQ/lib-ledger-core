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

#pragma once

#include <api/AlgorandNetworkParameters.hpp>
#include "model/transactions/AlgorandTransaction.hpp"
#include "model/transactions/AlgorandTransactionParams.hpp"
#include "model/AlgorandAccount.hpp"

#include <async/DedicatedContext.hpp>
#include <utils/ConfigurationMatchable.h>

#include <api/Block.hpp>
#include <net/HttpClient.hpp>

namespace ledger {
namespace core {
namespace algorand {
namespace constants {

    static const std::string purestakeTokenHeader = "x-api-key";

    // Explorer endpoints
    static const std::string purestakeStatusEndpoint = "/status";
    static const std::string purestakeBlockEndpoint = "/block/{}";
    static const std::string purestakeAccountEndpoint = "/account/{}";
    static const std::string purestakeAccountTransactionsEndpoint = "/account/{}/transactions";
    static const std::string purestakeTransactionEndpoint = "/transaction/{}";
    static const std::string purestakeTransactionsEndpoint = "/transactions";
    static const std::string purestakeTransactionsParamsEndpoint = "/transactions/params";
    static const std::string purestakeAssetEndpoint = "/asset/{}";

} // namespace constants

    class BlockchainExplorer : public ConfigurationMatchable, public DedicatedContext
    {

    public:

        BlockchainExplorer(const std::shared_ptr<api::ExecutionContext> & context,
                           const std::shared_ptr<HttpClient> & http,
                           const api::AlgorandNetworkParameters & parameters,
                           const std::shared_ptr<api::DynamicObject> & configuration);

        Future<api::Block> getBlock(uint64_t blockHeight) const;

        Future<api::Block> getLatestBlock() const;

        Future<model::Account> getAccount(const std::string & address) const;

        Future<model::AssetParams> getAssetById(uint64_t id) const;

        Future<model::Transaction> getTransactionById(const std::string & txId) const;

        Future<model::TransactionsBulk> getTransactionsForAddress(const std::string & address,
                                                                  const Option<uint64_t> & firstRound = Option<uint64_t>(),
                                                                  const Option<uint64_t> & lastRound = Option<uint64_t>()) const;

        Future<model::TransactionParams> getTransactionParams() const;

        Future<std::string> pushTransaction(const std::vector<uint8_t> & transaction);

    private:

        std::shared_ptr<HttpClient> _http;
        api::AlgorandNetworkParameters _parameters;
    };

} // namespace algorand
} // namespace core
} // namespace ledger

