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

#include <algorand/AlgorandBlockchainExplorer.hpp>
#include <algorand/AlgorandJsonParser.hpp>

#include <core/net/HttpClient.hpp>
#include <core/api/Configuration.hpp>

// TODO Tests

namespace ledger {
namespace core {
namespace algorand {

    BlockchainExplorer::BlockchainExplorer(const std::shared_ptr<api::ExecutionContext> &context,
                       const std::shared_ptr<HttpClient> &http,
                       const api::AlgorandNetworkParameters &parameters,
                       const std::shared_ptr<api::DynamicObject> &configuration) :
        ConfigurationMatchable({api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}),
        DedicatedContext(context),
        _http(http),
        _parameters(parameters) {
            setConfiguration(configuration);
        }

    FuturePtr<api::Block> BlockchainExplorer::getCurrentBlock() const {
        // TODO ?
    }

    FuturePtr<api::Block> BlockchainExplorer::getBlock(uint64_t &blockHeight) const {
        return _http->GET(fmt::format(constants::purestakeBlockEndpoint, blockHeight))
            .json(false)
            .mapPtr<api::Block>(getContext(), [] (const HttpRequest::JsonResult& response) {
                auto block = std::make_shared<api::Block>();
                const auto& json = std::get<1>(response)->GetObject();
                JsonParser::parseBlock(json, currencies::algorand().name, *block);
                return block;
            });
    }

    FuturePtr<model::Account> BlockchainExplorer::getAccount(const std::string & address) const {
        const bool parseJsonNumbersAsStrings = false; // FIXME Is this safe?

        return _http->GET(fmt::format(constants::purestakeAccountEndpoint, address))
            .json(false)
            .template flatMapPtr<model::Account>(
                getContext(),
                [] (const HttpRequest::JsonResult &response) {
                    auto account = model::Account();
                    const auto &json = std::get<1>(response)->GetObject();
                    JsonParser::parseAccount(json, account);
                    return FuturePtr<model::Account>::successful(std::make_shared<model::Account>(account));
                });
    }

    FuturePtr<model::Transaction>
    BlockchainExplorer::getTransactionById(const std::string & txId) const {
        return _http->GET(fmt::format(constants::purestakeTransactionEndpoint, txId))
            .json(false)
            .mapPtr<model::Transaction>(getContext(), [] (const HttpRequest::JsonResult& response) {
                const auto& json = std::get<1>(response)->GetObject();
                auto tx = std::make_shared<model::Transaction>();
                JsonParser::parseTransaction(json, *tx);
                return tx;
            });
    }

    FuturePtr<model::TransactionsBulk>
    BlockchainExplorer::getTransactionsForAddress(const std::string & address, const uint64_t & fromBlockHeight) const {
        return _http->GET(fmt::format(constants::purestakeAccountTransactionsEndpoint, address))
            .json(false)
            .mapPtr<model::TransactionsBulk>(getContext(), [] (const HttpRequest::JsonResult& response) {
                const auto& json = std::get<1>(response)->GetObject()[constants::transactions.c_str()].GetArray();
                auto tx = std::make_shared<model::TransactionsBulk>();
                JsonParser::parseTransactions(json, tx->transactions);
                // TODO Manage tx->hasNext ? Pagination ?
                return tx;
            });
    }

    Future<uint64_t> BlockchainExplorer::getSuggestedFee(const std::shared_ptr<api::AlgorandTransaction> &transaction) const {
        const auto txBytes = transaction->serialize();
        return _http->GET(constants::purestakeTransactionsParamsEndpoint)
            .json(false)
            .map<uint64_t>(getContext(), [&txBytes] (const HttpRequest::JsonResult& response) -> uint64_t {
                const auto& json = std::get<1>(response)->GetObject();
                const auto minimumFee = json[constants::minFee.c_str()].GetUint64();
                // FIXME May need to apply a majoration coefficient here,
                // because tx is missing fields signature and fee at this point
                const auto recommendedFee = txBytes.size() * json[constants::fee.c_str()].GetUint64();
                return std::max(minimumFee, recommendedFee);
            });
    }

    FuturePtr<model::TransactionParams> BlockchainExplorer::getTransactionParams() const {
        return _http->GET(constants::purestakeTransactionsParamsEndpoint)
            .json(false)
            .mapPtr<model::TransactionParams>(getContext(), [] (const HttpRequest::JsonResult& response) {
                const auto& json = std::get<1>(response)->GetObject();
                auto txParams = std::make_shared<model::TransactionParams>();
                JsonParser::parseTransactionParams(json, *txParams);
                return txParams;
            });
    }

    Future<std::string> BlockchainExplorer::pushTransaction(const std::vector<uint8_t> & transaction) {
        return _http->POST(constants::purestakeTransactionsEndpoint, transaction)
            .json(false)
            .template map<std::string>(_executionContext, [] (const HttpRequest::JsonResult& response) -> std::string {
                return std::get<1>(response)->GetObject()[constants::txId.c_str()].GetString();
            });
    }

}  // namespace algorand
}  // namespace core
}  // namespace ledger
