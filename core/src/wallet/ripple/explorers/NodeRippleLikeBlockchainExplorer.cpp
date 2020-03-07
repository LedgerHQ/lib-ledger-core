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
#include <wallet/currencies.hpp>

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
            auto size = addresses.size();
            if (size != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get balance of 1 address from Ripple Node, but got {} addresses", addresses.size());
            }
            std::string addressesStr = addresses[0]->toBase58();
            return getAccountInfo(addressesStr, "Balance", BigInt::ZERO);
        }

        Future<std::shared_ptr<BigInt>>
        NodeRippleLikeBlockchainExplorer::getSequence(const std::string &address) {
            return getAccountInfo(address, "Sequence", BigInt::ZERO);
        }

        Future<std::shared_ptr<BigInt>>
        NodeRippleLikeBlockchainExplorer::getFees() {
            return getServerState("base_fee");
        }

        Future<std::shared_ptr<BigInt>>
        NodeRippleLikeBlockchainExplorer::getBaseReserve() {
            return getServerState("reserve_base");
        }

        Future<std::shared_ptr<BigInt>>
        NodeRippleLikeBlockchainExplorer::getLedgerSequence() {
            return getServerState("seq");
        }

        Future<std::shared_ptr<BigInt>>
        NodeRippleLikeBlockchainExplorer::getServerState(const std::string &field) {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("server_state");
            auto requestBody = bodyRequest.getString();
            bool parseNumberAsString = true;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .json(parseNumberAsString).mapPtr<BigInt>(getContext(), [field](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        //Is there a result field ?
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get base reserve from network, no (or malformed) field \"result\" in response");
                        }

                        //Is there an info field ?
                        auto resultObj = json["result"].GetObject();
                        if (!resultObj.HasMember("state") || !resultObj["state"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get base reserve from network, no (or malformed) field \"state\" in response");
                        }

                        //Is there a validated_ledger field ?
                        auto infoObj = resultObj["state"].GetObject();
                        if (!infoObj.HasMember("validated_ledger") || !infoObj["validated_ledger"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get base reserve from network, no (or malformed) field \"validated_ledger\" in response");
                        }

                        auto reserveObj = infoObj["validated_ledger"].GetObject();
                        if (!reserveObj.HasMember(field.c_str()) || !reserveObj[field.c_str()].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 fmt::format("Failed to get {} from network, no (or malformed) field \"{}\" in response", field, field));
                        }
                        auto value = reserveObj[field.c_str()].GetString();
                        return std::make_shared<BigInt>(value);
                    });
        }

        Future<String>
        NodeRippleLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("submit");
            bodyRequest.pushParameter("tx_blob", hex::toString(transaction));
            auto requestBody = bodyRequest.getString();
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .json().template map<String>(getExplorerContext(), [](const HttpRequest::JsonResult &result) -> String {
                        auto &json = *std::get<1>(result);
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to broadcast transaction, no (or malformed) field \"result\" in response");
                        }

                        auto resultObj = json["result"].GetObject();

                        if (resultObj.HasMember("engine_result") &&
                            (resultObj["engine_result"] == "tesSUCCESS" ||
                             resultObj["engine_result"] == "terQUEUED")) {
                          // Check presence of tx_json field
                          if (!resultObj.HasMember("tx_json") || !resultObj["tx_json"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to broadcast transaction, no (or malformed) field \"tx_json\" in response");
                          }
                          auto txnObj = resultObj["tx_json"].GetObject();

                          // Check presence of hash field
                          if (!txnObj.HasMember("hash") || !txnObj["hash"].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to broadcast transaction, no (or malformed) field \"hash\" in response");
                          }

                          return txnObj["hash"].GetString();
                        }


                        throw make_exception(api::ErrorCode::HTTP_ERROR,
                                             "Failed to broadcast transaction: {}",
                                             resultObj["engine_result"].GetString());
                    });
        }

        Future<void *> NodeRippleLikeBlockchainExplorer::startSession() {
            return Future<void *>::successful(new std::string("", 0));
        }

        Future<Unit> NodeRippleLikeBlockchainExplorer::killSession(void *session) {
            return Future<Unit>::successful(unit);
        }

        Future<Bytes> NodeRippleLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("tx");
            bodyRequest.pushParameter("transaction", transactionHash);
            bodyRequest.pushParameter("binary", "true");
            auto requestBody = bodyRequest.getString();
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
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

        Future<String> NodeRippleLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        FuturePtr<RippleLikeBlockchainExplorer::TransactionsBulk>
        NodeRippleLikeBlockchainExplorer::getTransactions(
            const std::vector<std::string> &addresses,
            Option<std::string> fromBlockHash,
            Option<void *> session
        ) {
            if (addresses.size() != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get transactions for 1 address from Ripple Node, but got {} addresses", addresses.size());
            }

            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("account_tx");
            bodyRequest.pushParameter("account", addresses[0]);

            // handle transaction pagination in the case we have a pagination marker, which happens
            // when a getTransactions returns a TransactionsBulk containing such a marker
            if (!_paginationMarker.empty()) {
                auto marker = strings::split(_paginationMarker, "-");
                bodyRequest.pushPagination(marker[0], marker[1]);
            }

            auto requestBody = bodyRequest.getString();
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};

            auto self = shared_from_this();

            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .template json<TransactionsBulk, Exception>(
                            LedgerApiParser<TransactionsBulk, RippleLikeTransactionsBulkParser>())
                    .template mapPtr<TransactionsBulk>(getExplorerContext(), [self, fromBlockHash](
                            const Either<Exception, std::shared_ptr<TransactionsBulk>> &result) {
                        if (result.isLeft()) {
                            // Only case where we should emit block not found error
                            if (!fromBlockHash.isEmpty() && result.getLeft().getErrorCode() == api::ErrorCode::HTTP_ERROR) {
                                throw make_exception(api::ErrorCode::BLOCK_NOT_FOUND, "Unable to find block with hash {}", fromBlockHash.getValue());
                            } else {
                                throw result.getLeft();
                            }
                        } else {
                            // handle pagination if a pagination marker is present
                            auto bulk = result.getRight();

                            if (!bulk->paginationMarker.empty()) {
                                bulk->hasNext = true;
                                self->_paginationMarker = bulk->paginationMarker;
                            } else {
                                self->_paginationMarker = "";
                            }

                            return bulk;
                        }
                    });
        }

        FuturePtr<Block> NodeRippleLikeBlockchainExplorer::getCurrentBlock() const {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("ledger");
            bodyRequest.pushParameter("ledger_index", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .template json<Block, Exception>(LedgerApiParser<Block, RippleLikeBlockParser>())
                    .template mapPtr<Block>(getExplorerContext(),
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


        Future<std::shared_ptr<BigInt>>
        NodeRippleLikeBlockchainExplorer::getAccountInfo(const std::string &address,
                                                         const std::string &key,
                                                         const BigInt &defaultValue) {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("account_info");
            bodyRequest.pushParameter("account", address);
            bodyRequest.pushParameter("ledger_index", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            bool parseNumberAsString = true;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .json(parseNumberAsString).mapPtr<BigInt>(getContext(), [address, key, defaultValue](const HttpRequest::JsonResult &result) {
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
                            // Case of account not found (not activated yet)
                            return std::make_shared<BigInt>(defaultValue);
                        }

                        //Is there an field with key name ?
                        auto accountObj = resultObj["account_data"].GetObject();
                        if (!accountObj.HasMember(key.c_str()) || !accountObj[key.c_str()].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get {} for {}, no (or malformed) field \"{}\" in response",
                                                 key, address, key);
                        }

                        auto value = accountObj[key.c_str()].GetString();
                        return std::make_shared<BigInt>(value);
                    });
        }
    }
}
