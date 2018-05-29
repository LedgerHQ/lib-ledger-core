/*
 *
 * LedgerApiBitcoinLikeBlockchainExplorer
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/03/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "LedgerApiBitcoinLikeBlockchainExplorer.hpp"
#include <fmt/format.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <api/Configuration.hpp>
#include "../../../utils/hex.h"
#include "api/TransactionsBulkParser.hpp"
#include "api/TransactionParser.hpp"
#include "api/BlockParser.hpp"
#include "api/LedgerApiParser.hpp"
#include <utils/JSONUtils.h>
#include <sstream>

namespace ledger {
    namespace core {

        LedgerApiBitcoinLikeBlockchainExplorer::LedgerApiBitcoinLikeBlockchainExplorer(
        const std::shared_ptr<api::ExecutionContext> &context,
        const std::shared_ptr<HttpClient> &http,
        const api::BitcoinLikeNetworkParameters& parameters,
        const std::shared_ptr<api::DynamicObject>& configuration) :
                DedicatedContext(context),
                BitcoinLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http = http;
            _parameters = parameters;
        }

        Future<void *> LedgerApiBitcoinLikeBlockchainExplorer::startSession() {
            return _http
            ->GET(fmt::format("/blockchain/v2/{}/syncToken", _parameters.Identifier))
            .json().map<void *>(getContext(), [] (const HttpRequest::JsonResult& result) {
                auto& json = *std::get<1>(result);
                return new std::string(json["token"].GetString(), json["token"].GetStringLength());
            });
        }

        Future<Unit> LedgerApiBitcoinLikeBlockchainExplorer::killSession(void *session) {
            return _http
            ->addHeader("X-LedgerWallet-SyncToken", *((std::string *)session))
            .DEL(fmt::format("/blockchain/v2/{}/syncToken", _parameters.Identifier))
            .json().map<Unit>(getContext(), [] (const HttpRequest::JsonResult& result) {
                return unit;
            });
        }

        Future<String> LedgerApiBitcoinLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            std::stringstream body;
            body << "{" << "\"tx\":" << '"' << hex::toString(transaction) << '"' << "}";
            auto bodyString = body.str();
            return _http
                ->POST(fmt::format("/blockchain/v2/{}/transactions/send", _parameters.Identifier),
                    std::vector<uint8_t>(bodyString.begin(), bodyString.end())
                ).json().map<String>(getContext(), [] (const HttpRequest::JsonResult& result) -> String {
                auto& json = *std::get<1>(result);
                return json["result"].GetString();
            });
        }

        Future<Bytes> LedgerApiBitcoinLikeBlockchainExplorer::getRawTransaction(const String& transactionHash) {
            return _http
            ->GET(fmt::format("/blockchain/v2/{}/transactions/{}/hex", _parameters.Identifier, transactionHash.str()))
            .json().map<Bytes>(getContext(), [transactionHash] (const HttpRequest::JsonResult& result) {
                auto& json = *std::get<1>(result);
                if (json.GetArray().Size() == 0) {
                    throw make_exception(api::ErrorCode::RAW_TRANSACTION_NOT_FOUND, "Unable to retrieve {}", transactionHash.str());
                } else {
                    auto& hex = json[0].GetObject()["hex"];
                    return Bytes(ledger::core::hex::toByteArray(std::string(hex.GetString(), hex.GetStringLength())));
                }
            });
        }

        FuturePtr<BitcoinLikeBlockchainExplorer::TransactionsBulk>
        LedgerApiBitcoinLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                                Option<std::string> fromBlockHash,
                                                                Option<void *> session) {
            auto joinedAddresses = Array<std::string>(addresses).join(strings::mkString(",")).getValueOr("");
            std::string params;
            std::unordered_map<std::string, std::string> headers;

            if (session.isEmpty()) {
                params = "?noToken=true";
            } else {
                headers["X-LedgerWallet-SyncToken"] = *((std::string *)session.getValue());
            }
            if (!fromBlockHash.isEmpty()) {
                if (params.size() > 0) {
                    params = params + "&";
                } else {
                    params = params + "?";
                }
                params = params + "blockHash=" + fromBlockHash.getValue();
            }
            return _http
            ->GET(fmt::format("/blockchain/v2/{}/addresses/{}/transactions{}", _parameters.Identifier, joinedAddresses, params), headers)
            .json<BitcoinLikeBlockchainExplorer::TransactionsBulk, Exception>(LedgerApiParser<BitcoinLikeBlockchainExplorer::TransactionsBulk, TransactionsBulkParser>())
            .mapPtr<TransactionsBulk>(_executionContext, [] (const Either<Exception, std::shared_ptr<BitcoinLikeBlockchainExplorer::TransactionsBulk>>& result) {
                if (result.isLeft()) {
                    throw result.getLeft();
                } else {
                    return result.getRight();
                }
            });
        }

        FuturePtr<BitcoinLikeBlockchainExplorer::Block> LedgerApiBitcoinLikeBlockchainExplorer::getCurrentBlock() {
            return _http
            ->GET(fmt::format("/blockchain/v2/{}/blocks/current", _parameters.Identifier))
            .json<BitcoinLikeBlockchainExplorer::Block, Exception>(LedgerApiParser<BitcoinLikeBlockchainExplorer::Block, BlockParser>())
            .mapPtr<BitcoinLikeBlockchainExplorer::Block>(_executionContext, [] (const Either<Exception, std::shared_ptr<BitcoinLikeBlockchainExplorer::Block>>& result) {
                if (result.isLeft()) {
                    throw result.getLeft();
                } else {
                    return result.getRight();
                }
            });
        }

        FuturePtr<BitcoinLikeBlockchainExplorer::Transaction>
        LedgerApiBitcoinLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) {
            return _http
                ->GET(fmt::format("/blockchain/v2/{}/transactions/{}", _parameters.Identifier, transactionHash.str()))
                .json<std::vector<BitcoinLikeBlockchainExplorer::Transaction>, Exception>(LedgerApiParser<std::vector<BitcoinLikeBlockchainExplorer::Transaction>, TransactionsParser>())
            .mapPtr<BitcoinLikeBlockchainExplorer::Transaction>(_executionContext, [transactionHash] (const Either<Exception, std::shared_ptr<std::vector<BitcoinLikeBlockchainExplorer::Transaction>>>& result) {
                if (result.isLeft()) {
                    throw result.getLeft();
                } else if (result.getRight()->size() == 0) {
                    throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction '{}' not found", transactionHash.str());
                } else {
                    auto tx = (*result.getRight())[0];
                    auto transaction = std::make_shared<BitcoinLikeBlockchainExplorer::Transaction>();
                    transaction->block = tx.block;
                    transaction->fees = tx.fees;
                    transaction->hash = tx.hash;
                    transaction->lockTime = tx.lockTime;
                    transaction->inputs = tx.inputs;
                    transaction->outputs = tx.outputs;
                    transaction->receivedAt = tx.receivedAt;
                    transaction->confirmations = tx.confirmations;
                    return transaction;
                }
            });
        }

        Future<int64_t > LedgerApiBitcoinLikeBlockchainExplorer::getTimestamp() {
            auto delay = 60*_parameters.TimestampDelay;
            return _http->GET(fmt::format("/timestamp"))
                    .json().map<int64_t>(getContext(), [delay] (const HttpRequest::JsonResult& result) {
                    auto& json = *std::get<1>(result);
                    return json["timestamp"].GetInt64() - delay;
            });

        }

    }
}