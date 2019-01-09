/*
 *
 * NodeRippleLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 09/01/2019.
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


#include "NodeRippleLikeBlockchainExplorer.h"
#include <api/RippleConfigurationDefaults.hpp>
#include <api/Configuration.hpp>
#include <rapidjson/document.h>
namespace ledger {
    namespace core {
        NodeRippleLikeBlockchainExplorer::NodeRippleLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http,
                const api::RippleLikeNetworkParameters &parameters,
                const std::shared_ptr<api::DynamicObject> &configuration) :
                DedicatedContext(context),
                RippleLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http = http;
            _parameters = parameters;
        }

        Future<std::shared_ptr<BigInt>>
        NodeRippleLikeBlockchainExplorer::getBalance(const std::vector<RippleLikeKeychain::Address> &addresses) {
            //TODO: multiple accounts balances ?
            auto size = addresses.size();
            if (size != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get balance of 1 address from Ripple Node, but got {} addresses", addresses.size());
            }
            std::string addressesStr = addresses[0]->toBase58();
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("account_info");
            bodyRequest.pushParameter("account", addressesStr);
            bodyRequest.pushParameter("ledger_index", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .json().mapPtr<BigInt>(getContext(), [addressesStr, size](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        //Is there a result field ?
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get balance for {}, no (or malformed) field \"result\" in response",
                                                 addressesStr);
                        }
                        //Is there an account_data field ?
                        auto resultObj = json["result"].GetObject();
                        if (!resultObj.HasMember("account_data") || !resultObj["account_data"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get balance for {}, no (or malformed) field \"account_data\" in response",
                                                 addressesStr);
                        }
                        //Is there an Balance field ?
                        auto accountObj = resultObj["account_data"].GetObject();
                        if (!accountObj.HasMember("Balance") || !accountObj["Balance"].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get balance for {}, no (or malformed) field \"Balance\" in response",
                                                 addressesStr);
                        }
                        auto balance = accountObj["Balance"].GetString();
                        return std::make_shared<BigInt>(balance);
                    });
        }

        Future<String>
        NodeRippleLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            std::stringstream body;
            body << "{" << "\"tx\":" << '"' << hex::toString(transaction) << '"' << "}";
            auto bodyString = body.str();
            return _http->POST(fmt::format("/blockchain/{}/{}/transactions/send",
                                           getNetworkParameters().Identifier),
                               std::vector<uint8_t>(bodyString.begin(), bodyString.end())
            ).json().template map<String>(getExplorerContext(), [](const HttpRequest::JsonResult &result) -> String {
                auto &json = *std::get<1>(result);
                return json["result"].GetString();
            });
        }

        Future<void *> NodeRippleLikeBlockchainExplorer::startSession() {
            return Future<void *>::successful(new std::string("", 0));
        }

        Future<Unit> NodeRippleLikeBlockchainExplorer::killSession(void *session) {
            return Future<Unit>::successful(unit);
        }

        Future<Bytes> NodeRippleLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            return getLedgerApiRawTransaction(transactionHash);
        }

        Future<String> NodeRippleLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        FuturePtr<RippleLikeBlockchainExplorer::TransactionsBulk>
        NodeRippleLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                          Option<std::string> fromBlockHash,
                                                          Option<void *> session) {
            if (addresses.size() != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get transactions for 1 address from Ripple Node, but got {} addresses", addresses.size());
            }
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("account_tx");
            bodyRequest.pushParameter("account", addresses[0]);
            bodyRequest.pushParameter("ledger_index", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .template json<TransactionsBulk, Exception>(
                            LedgerApiParser<TransactionsBulk, RippleLikeTransactionsBulkParser>())
                    .
                            template mapPtr<TransactionsBulk>(getExplorerContext(), [fromBlockHash](
                            const Either<Exception, std::shared_ptr<TransactionsBulk>> &result) {
                        if (result.isLeft()) {
                            if (fromBlockHash.isEmpty()) {
                                throw result.getLeft();
                            } else {
                                throw make_exception(api::ErrorCode::BLOCK_NOT_FOUND,
                                                     "Unable to find block with hash {}", fromBlockHash.getValue());
                            }
                        } else {
                            return result.getRight();
                        }
                    });
        }

        FuturePtr<Block> NodeRippleLikeBlockchainExplorer::getCurrentBlock() const {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("ledger");
            bodyRequest.pushParameter("ledger_index", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .template json<Block, Exception>(LedgerApiParser<Block, RippleLikeBlockParser>())
                    .
                            template mapPtr<Block>(getExplorerContext(),
                                                   [](const Either<Exception, std::shared_ptr<Block>> &result) {
                                                       if (result.isLeft()) {
                                                           throw result.getLeft();
                                                       } else {
                                                           return result.getRight();
                                                       }
                                                   });
        }

        FuturePtr<RippleLikeBlockchainExplorerTransaction>
        NodeRippleLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t> NodeRippleLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> NodeRippleLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::RippleLikeNetworkParameters NodeRippleLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string NodeRippleLikeBlockchainExplorer::getExplorerVersion() const {
            return "";
        }
    }
}