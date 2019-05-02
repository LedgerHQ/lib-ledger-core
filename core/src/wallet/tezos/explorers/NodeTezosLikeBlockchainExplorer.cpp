/*
 *
 * NodeTezosLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 29/04/2019.
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


#include "NodeTezosLikeBlockchainExplorer.h"
#include <api/TezosConfigurationDefaults.hpp>
#include <api/Configuration.hpp>
#include <rapidjson/document.h>
namespace ledger {
    namespace core {
        NodeTezosLikeBlockchainExplorer::NodeTezosLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http,
                const api::TezosLikeNetworkParameters &parameters,
                const std::shared_ptr<api::DynamicObject> &configuration) :
                DedicatedContext(context),
                TezosLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http = http;
            _parameters = parameters;
        }


        Future<std::shared_ptr<BigInt>>
        NodeTezosLikeBlockchainExplorer::getBalance(const std::vector<TezosLikeKeychain::Address> &addresses) {
            auto size = addresses.size();
            if (size != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get balance of 1 address from Tezos Node, but got {} addresses", addresses.size());
            }
            std::string addressesStr = addresses[0]->toBase58();
            return getAccountInfo(addressesStr, "Balance", FieldTypes::StringType);
        }

        Future<std::shared_ptr<BigInt>>
        NodeTezosLikeBlockchainExplorer::getFees() {
            NodeTezosLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("fee");
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .json().mapPtr<BigInt>(getContext(), [](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        //Is there a result field ?
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get fees from network, no (or malformed) field \"result\" in response");
                        }

                        auto resultObj = json["result"].GetObject();
                        //Is there a drops field ?
                        if (!resultObj.HasMember("drops") || !resultObj["drops"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get fees from network, no (or malformed) field \"drops\" in response");
                        }

                        //Is there a base_fee field ?
                        auto dropObj = resultObj["drops"].GetObject();
                        if (!dropObj.HasMember("base_fee") || !dropObj["base_fee"].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get fees from network, no (or malformed) field \"base_fee\" in response");
                        }

                        auto fees = dropObj["base_fee"].GetString();
                        return std::make_shared<BigInt>(fees);
                    });
        }

        Future<String>
        NodeTezosLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            NodeTezosLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("submit");
            bodyRequest.pushParameter("tx_blob", hex::toString(transaction));
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .json().template map<String>(getExplorerContext(), [](const HttpRequest::JsonResult &result) -> String {
                        auto &json = *std::get<1>(result);
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to broadcast transaction, no (or malformed) field \"result\" in response");
                        }
                        //Is there an account_data field ?
                        auto resultObj = json["result"].GetObject();
                        if (!resultObj.HasMember("hash") || !resultObj["hash"].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to broadcast transaction, no (or malformed) field \"hash\" in response");
                        }
                        return resultObj["hash"].GetString();
                    });
        }

        Future<void *> NodeTezosLikeBlockchainExplorer::startSession() {
            return Future<void *>::successful(new std::string("", 0));
        }

        Future<Unit> NodeTezosLikeBlockchainExplorer::killSession(void *session) {
            return Future<Unit>::successful(unit);
        }

        Future<Bytes> NodeTezosLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            NodeTezosLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("tx");
            bodyRequest.pushParameter("trasnaction", transactionHash);
            bodyRequest.pushParameter("binary", "true");
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .json().template map<Bytes>(getExplorerContext(), [](const HttpRequest::JsonResult &result) -> Bytes {
                        auto &json = *std::get<1>(result);
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get raw transaction, no (or malformed) field \"result\" in response");
                        }
                        //Is there a tx field ?
                        auto resultObj = json["result"].GetObject();
                        if (!resultObj.HasMember("tx") || !resultObj["tx"].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get raw transaction, no (or malformed) field \"tx\" in response");
                        }
                        return Bytes(hex::toByteArray(std::string(resultObj["tx"].GetString(), resultObj["tx"].GetStringLength())));
                    });
        }

        Future<String> NodeTezosLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        FuturePtr<TezosLikeBlockchainExplorer::TransactionsBulk>
        NodeTezosLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                          Option<std::string> fromBlockHash,
                                                          Option<void *> session) {
            if (addresses.size() != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get transactions for 1 address from Tezos Node, but got {} addresses", addresses.size());
            }
            NodeTezosLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("account_tx");
            bodyRequest.pushParameter("account", addresses[0]);
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .template json<TransactionsBulk, Exception>(
                            LedgerApiParser<TransactionsBulk, TezosLikeTransactionsBulkParser>())
                    .template mapPtr<TransactionsBulk>(getExplorerContext(), [fromBlockHash](
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

        FuturePtr<Block> NodeTezosLikeBlockchainExplorer::getCurrentBlock() const {
            NodeTezosLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("block");
            bodyRequest.pushParameter("block_height", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .template json<Block, Exception>(LedgerApiParser<Block, TezosLikeBlockParser>())
                    .template mapPtr<Block>(getExplorerContext(),
                                            [](const Either<Exception, std::shared_ptr<Block>> &result) {
                                                if (result.isLeft()) {
                                                    throw result.getLeft();
                                                } else {
                                                    return result.getRight();
                                                }
                                            });
        }

        FuturePtr<TezosLikeBlockchainExplorerTransaction>
        NodeTezosLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t> NodeTezosLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> NodeTezosLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::TezosLikeNetworkParameters NodeTezosLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string NodeTezosLikeBlockchainExplorer::getExplorerVersion() const {
            return "";
        }


        Future<std::shared_ptr<BigInt>>
        NodeTezosLikeBlockchainExplorer::getAccountInfo(const std::string &address,
                                                         const std::string &key,
                                                         FieldTypes type) {
            NodeTezosLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("account_info");
            bodyRequest.pushParameter("account", address);
            bodyRequest.pushParameter("block_height", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()))
                    .json().mapPtr<BigInt>(getContext(), [address, key, type](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        //Is there a result field ?
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get {} for {}, no (or malformed) field \"result\" in response",
                                                 key, address);
                        }

                        //Is there an account_data field ?
                        auto resultObj = json["result"].GetObject();
                        if (!resultObj.HasMember("account_data") || !resultObj["account_data"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get {} for {}, no (or malformed) field \"account_data\" in response",
                                                 key, address);
                        }

                        //Is there an field with key name ?
                        auto accountObj = resultObj["account_data"].GetObject();
                        switch(type) {
                            case FieldTypes::NumberType : {
                                if (!accountObj.HasMember(key.c_str()) || !accountObj[key.c_str()].IsUint64()) {
                                    throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get {} for {}, no (or malformed) field \"{}\" in response",
                                                         key, address, key);
                                }

                                auto balance = accountObj[key.c_str()].GetUint64();
                                return std::make_shared<BigInt>((int64_t)balance);
                            }
                            case FieldTypes::StringType : {
                                if (!accountObj.HasMember(key.c_str()) || !accountObj[key.c_str()].IsString()) {
                                    throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get {} for {}, no (or malformed) field \"{}\" in response",
                                                         key, address, key);
                                }

                                auto balance = accountObj[key.c_str()].GetString();
                                return std::make_shared<BigInt>(balance);
                            }
                        }

                        throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Failed to get {} which has an unkown field type",
                                             key);
                    });
        }
    }
}