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

#include "AlgorandBlockchainExplorer.hpp"
#include "AlgorandJsonParser.hpp"
#include "AlgorandExplorerConstants.hpp"

#include <api/Configuration.hpp>
#include <wallet/common/api_impl/OperationApi.h>


namespace ledger {
namespace core {
namespace algorand {

    namespace constants {

        const std::string purestakeTokenHeader = "x-api-key";

        // Explorer endpoints
        const std::string purestakeStatusEndpoint = "/ps2/v2/status";
        const std::string purestakeTransactionsEndpoint = "/ps2/v2/transactions";
        const std::string purestakeTransactionsParamsEndpoint = "/ps2/v2/transactions/params";
        const std::string purestakeAccountEndpoint = "/ps2/v2/accounts/{}";
        const std::string purestakeBlockEndpoint = "/ps2/v2/blocks/{}?format=json";
        const std::string purestakeAccountTransactionsEndpoint = "/idx2/v2/accounts/{}/transactions";
        const std::string purestakeTransactionEndpoint = "/idx2/v2/transactions?txid={}";
        const std::string purestakeAssetEndpoint = "/idx2/v2/assets/{}";

        // Query parameters
        const std::string limitQueryParam = "{}?limit={}";
        const std::string minRoundQueryParam = "{}&min-round={}";
        const std::string maxRoundQueryParam = "{}&max-round={}";

    } // namespace constants

    BlockchainExplorer::BlockchainExplorer(
            const std::shared_ptr<api::ExecutionContext>& context,
            const std::shared_ptr<HttpClient>& http,
            const api::AlgorandNetworkParameters& parameters,
            const std::shared_ptr<api::DynamicObject>& configuration)
        : ConfigurationMatchable({
            api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT,
            api::Configuration::BLOCKCHAIN_EXPLORER_API_KEY
            })
        , DedicatedContext(context)
        , _http(http)
        , _parameters(parameters)
    {
        setConfiguration(configuration);
        const auto apiKey = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_KEY);
        if (apiKey && !apiKey.value().empty()) {
            _http->addHeader(constants::purestakeTokenHeader, apiKey.value());
        }
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

    Future<api::Block> BlockchainExplorer::getLatestBlock() const
    {
        // The Algorand API doesn't have a "latest block" endpoint,
        // so we need to retrieve the latest round number first,
        // then retrieve the associated block
        return _http->GET(constants::purestakeStatusEndpoint)
            .json(false)
            .map<uint64_t>(getContext(), [](const HttpRequest::JsonResult &response) {
                    const auto &json = std::get<1>(response)->GetObject();
                    if (!json.HasMember(constants::xLastRoundParam.c_str())) {
                        throw make_exception(api::ErrorCode::NO_SUCH_ELEMENT, fmt::format("Missing '{}' field in JSON.", constants::xLastRoundParam));
                    }
                    return json[constants::xLastRoundParam.c_str()].GetUint64();
            }).flatMap<api::Block>(getContext(), [this](uint64_t latestRound) {
                    return getBlock(latestRound);
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
                    [id](const HttpRequest::JsonResult& response) {
                        auto assetParams = model::AssetParams();
                        const auto& json = std::get<1>(response)->GetObject();
                        if (!json.HasMember(constants::xAsset.c_str())) {
                            throw make_exception(api::ErrorCode::CURRENCY_NOT_FOUND,
                                                 fmt::format("Asset {} does not exist", id));
                        }
                        const auto& jsonParams =
                            json[constants::xAsset.c_str()].GetObject()[constants::xParams.c_str()].GetObject();
                        JsonParser::parseAssetParams(jsonParams, assetParams);
                        return assetParams;
                    });
    }

    Future<model::Transaction>
    BlockchainExplorer::getTransactionById(const std::string & txId) const {
        return _http->GET(fmt::format(constants::purestakeTransactionEndpoint, txId))
            .json(false)
            .map<model::Transaction>(getContext(), [txId](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    if (!json.HasMember(constants::xTransactions.c_str())) {
                        throw make_exception(api::ErrorCode::NO_SUCH_ELEMENT, fmt::format("Missing '{}' field in JSON.", constants::xTransactions));
                    }
                    const auto& jsonArray = json[constants::xTransactions.c_str()].GetArray();
                    if (jsonArray.Size() != 1) {
                        throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND,
                                             fmt::format("Couldn't find transaction {}", txId));
                    }
                    const auto& jsonTx = *std::begin(jsonArray);
                    auto tx = model::Transaction();
                    JsonParser::parseTransaction(jsonTx, tx);
                    return tx;
            });
    }

    Future<model::TransactionsBulk>
    BlockchainExplorer::getTransactionsForAddress(const std::string & address,
                                                  const Option<uint64_t> & firstRound,
                                                  const Option<uint64_t> & lastRound) const
    {
        auto url = fmt::format(constants::purestakeAccountTransactionsEndpoint, address);
        url = fmt::format(constants::limitQueryParam, url, constants::EXPLORER_QUERY_LIMIT);
        if (firstRound) {
            url = fmt::format(constants::minRoundQueryParam, url, *firstRound);
        }
        if (lastRound) {
            url = fmt::format(constants::maxRoundQueryParam, url, *lastRound);
        }

        return _http->GET(url)
            .json(false)
            .map<model::TransactionsBulk>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    if (!json.HasMember(constants::xTransactions.c_str())) {
                        throw make_exception(api::ErrorCode::NO_SUCH_ELEMENT, fmt::format("Missing '{}' field in JSON.", constants::xTransactions));
                    }
                    const auto& jsonArray = json[constants::xTransactions.c_str()].GetArray();
                    auto txs = model::TransactionsBulk();
                    JsonParser::parseTransactions(jsonArray, txs.transactions);

                    // Manage limit
                    txs.hasNext = txs.transactions.size() >= constants::EXPLORER_QUERY_LIMIT;
                    return txs;
            });
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

    Future<std::string> BlockchainExplorer::pushTransaction(const std::vector<uint8_t>& transaction, const std::string& correlationId)
    {
        static const std::unordered_map<std::string, std::string>
            CONTENT_TYPE_HEADER{{"Content-Type", "application/x-binary"}};

        return _http->POST(constants::purestakeTransactionsEndpoint, transaction, CONTENT_TYPE_HEADER)
            .json(false)
            .map<std::string>(_executionContext, [correlationId](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    if (!json.HasMember(constants::xTxId.c_str())) {
                        throw make_exception(api::ErrorCode::NO_SUCH_ELEMENT, fmt::format("{} Missing '{}' field in JSON.", 
                            CORRELATIONID_PREFIX(correlationId), constants::xTxId));
                    }
                    return json[constants::xTxId.c_str()].GetString();
            });
    }

}  // namespace algorand
}  // namespace core
}  // namespace ledger
