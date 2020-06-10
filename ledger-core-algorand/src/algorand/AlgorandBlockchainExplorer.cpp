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
#include <algorand/api/AlgorandConfigurationDefaults.hpp>

#include <core/api/Configuration.hpp>

namespace ledger {
namespace core {
namespace algorand {

    BlockchainExplorer::BlockchainExplorer(
            const std::shared_ptr<api::ExecutionContext>& context,
            const std::shared_ptr<HttpClient>& http,
            const api::AlgorandNetworkParameters& parameters,
            const std::shared_ptr<api::DynamicObject>& configuration)
        : ConfigurationMatchable({api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT})
        , DedicatedContext(context)
        , _http(http)
        , _parameters(parameters)
    {
        setConfiguration(configuration);
        _http->addHeader(constants::purestakeTokenHeader, api::AlgorandConfigurationDefaults::ALGORAND_API_TOKEN);
    }

    Future<api::Block> BlockchainExplorer::getBlock(uint64_t blockHeight) const
    {
        return _http->GET(fmt::format(constants::purestakeBlockEndpoint, blockHeight))
            .json(false)
            .map<api::Block>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto block = api::Block();
                    JsonParser::parseBlock(json, block);
                    return block;
            });
    }

    Future<model::Account> BlockchainExplorer::getAccount(const std::string& address) const
    {
        return _http->GET(fmt::format(constants::purestakeAccountEndpoint, address))
            .json(false)
            .map<model::Account>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto account = model::Account();
                    JsonParser::parseAccount(json, account);
                    return account;
            });
    }

    Future<model::AssetParams> BlockchainExplorer::getAssetById(uint64_t id) const
    {
        return _http->GET(fmt::format(constants::purestakeAssetEndpoint, id))
            .json(false)
            .map<model::AssetParams>(
                    getContext(),
                    [](const HttpRequest::JsonResult& response) {
                        auto assetParams = model::AssetParams();
                        const auto& json = std::get<1>(response)->GetObject();
                        JsonParser::parseAssetParams(json, assetParams);
                        return assetParams;
                    });
    }

    Future<model::Transaction>
    BlockchainExplorer::getTransactionById(const std::string & txId) const {
        return _http->GET(fmt::format(constants::purestakeTransactionEndpoint, txId))
            .json(false)
            .map<model::Transaction>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto tx = model::Transaction();
                    JsonParser::parseTransaction(json, tx);
                    return tx;
            });
            /*// NOTE: In case we actually need to retrieve blocks for each tx... (untested!)
            // Add block information to the transaction
            .template flatMap<model::Transaction>(getContext(), [this](const model::Transaction &tx) -> Future<model::Transaction> {
                auto output = model::Transaction(tx);
                return getBlock(tx.header.round.getValue())
                    .map<model::Transaction>(getContext(), [output](const api::Block &block) mutable {
                        output.header.block = block;
                        return output;
                    });
            });
            */
    }

    Future<model::TransactionsBulk>
    BlockchainExplorer::getTransactionsForAddress(const std::string & address,
                                                  const Option<uint64_t> & beforeRound) const
    {
        auto url = fmt::format(constants::purestakeAccountTransactionsEndpoint, address);
        // Apply the offset if required
        auto urlWithQueryFilters = beforeRound.isEmpty() ? url : fmt::format("{}?lastRound={}", url, beforeRound.getValue());

        return _http->GET(urlWithQueryFilters)
            .json(false)
            .map<model::TransactionsBulk>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject()[constants::xTransactions.c_str()].GetArray();
                    auto txs = model::TransactionsBulk();
                    JsonParser::parseTransactions(json, txs.transactions);

                    // Manage limit
                    txs.hasNext = txs.transactions.size() >= constants::EXPLORER_QUERY_LIMIT;
                    return txs;
            });
            /*// NOTE: In case we actually need to retrieve blocks for each tx... (untested!)
            .template flatMap<model::TransactionsBulk>(getContext(), [this](const model::TransactionsBulk & txs) -> Future<model::TransactionsBulk> {

                // - build set of (unique) blockHeights to fetch
                const auto blockHeights = vector::map<uint64_t, model::Transaction>(txs.transactions,
                    [](const model::Transaction& tx) {
                        return tx.header.round.getValue();
                    });
                const std::unordered_set<uint64_t> uniqueBlockHeights(blockHeights.begin(), blockHeights.end());

                // - build vector of promises for each block
                std::vector<Future<api::Block>> blockPromises;
                for (auto blockHeight : uniqueBlockHeights) {
                    blockPromises.push_back(getBlock(blockHeight));
                }

                // - async::sequence
                return async::sequence(getContext(), blockPromises)
                    .flatMap<model::TransactionsBulk>(getContext(), [&txs](const std::vector<api::Block> & blocks) {
                        // - build map of <blockHeight, api::Block> ?
                        //auto blockPerHeight = std::unordered_map<uint64_t, api::Block>();
                        auto txsMutable = const_cast<model::TransactionsBulk &>(txs);
                        for (auto& tx : txsMutable.transactions) {
                            for (const auto& block : blocks) {
                                if (tx.header.round.getValue() == block.height) {
                                    tx.header.block = block;
                                }
                            }
                        }

                        return Future<model::TransactionsBulk>::successful(txsMutable);
                    });
            });
            */
    }

    Future<model::TransactionParams> BlockchainExplorer::getTransactionParams() const
    {
        return _http->GET(constants::purestakeTransactionsParamsEndpoint)
            .json(false)
            .map<model::TransactionParams>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto txParams = model::TransactionParams();
                    JsonParser::parseTransactionParams(json, txParams);
                    return txParams;
            });
    }

    Future<std::string> BlockchainExplorer::pushTransaction(const std::vector<uint8_t>& transaction)
    {
        return _http->POST(constants::purestakeTransactionsEndpoint, transaction)
            .json(false)
            .map<std::string>(_executionContext, [](const HttpRequest::JsonResult& response) {
                    return std::get<1>(response)->GetObject()[constants::xTxId.c_str()].GetString();
            });
    }

}  // namespace algorand
}  // namespace core
}  // namespace ledger

