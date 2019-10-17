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


#include <rapidjson/document.h>

#include <core/api/Configuration.hpp>

#include <ripple/api/RippleConfigurationDefaults.hpp>
#include <ripple/explorers/NodeRippleLikeBlockchainExplorer.hpp>
#include <ripple/RippleLikeCurrencies.hpp>

namespace ledger {
    namespace core {
        class NodeRippleLikeBodyRequest {
        public:
            NodeRippleLikeBodyRequest() {
                //Document should be defined as object
                _document.SetObject();
                _params = rapidjson::Value(rapidjson::kObjectType);
            };

            NodeRippleLikeBodyRequest &setMethod(const std::string &method) {
                //In case need to allocate more memory
                rapidjson::Document::AllocatorType &allocator = _document.GetAllocator();
                //Field with method
                rapidjson::Value vMethod(rapidjson::kStringType);
                vMethod.SetString(method.c_str(), static_cast<rapidjson::SizeType>(method.length()), allocator);
                _document.AddMember("method", vMethod, allocator);
                return *this;
            };

            NodeRippleLikeBodyRequest &pushParameter(const std::string &key, const std::string &value) {
                rapidjson::Document::AllocatorType &allocator = _document.GetAllocator();
                rapidjson::Value vKeyParam(rapidjson::kStringType);
                vKeyParam.SetString(key.c_str(), static_cast<rapidjson::SizeType>(key.length()), allocator);
                rapidjson::Value vParam(rapidjson::kStringType);
                vParam.SetString(value.c_str(), static_cast<rapidjson::SizeType>(value.length()), allocator);
                _params.AddMember(vKeyParam, vParam, allocator);
                return *this;
            };

            NodeRippleLikeBodyRequest &pushParameter(const std::string &key, int64_t value) {
                rapidjson::Document::AllocatorType &allocator = _document.GetAllocator();
                rapidjson::Value vKeyParam(rapidjson::kStringType);
                vKeyParam.SetString(key.c_str(), static_cast<rapidjson::SizeType>(key.length()), allocator);
                rapidjson::Value vParam(rapidjson::kNumberType);
                vParam.SetInt64(value);
                _params.AddMember(vKeyParam, vParam, allocator);
                return *this;
            };

            std::string getString() {
                rapidjson::Document::AllocatorType &allocator = _document.GetAllocator();
                rapidjson::Value container(rapidjson::kArrayType);
                container.PushBack(_params, allocator);
                _document.AddMember("params", container, allocator);
                //Stream to string buffer
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                _document.Accept(writer);
                return buffer.GetString();
            };

        private:
            rapidjson::Document _document;
            rapidjson::Value _params;
        };

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

        Future<std::shared_ptr<BigInt>> NodeRippleLikeBlockchainExplorer::getBalance(
            const std::vector<RippleLikeKeychain::Address> &addresses
        ) {
            auto size = addresses.size();
            if (size != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get balance of 1 address from Ripple Node, but got {} addresses", addresses.size());
            }
            std::string addressesStr = addresses[0]->toBase58();
            return getAccountInfo(addressesStr, "Balance", BigInt::ZERO, FieldTypes::StringType);
        }

        Future<std::shared_ptr<BigInt>> NodeRippleLikeBlockchainExplorer::getSequence(
            const std::string &address
        ) {
            return getAccountInfo(address, "Sequence", BigInt::ZERO, FieldTypes::NumberType);
        }

        Future<std::shared_ptr<BigInt>> NodeRippleLikeBlockchainExplorer::getFees() {
            return getServerInfo("base_fee_xrp", FieldTypes::NumberType);
        }

        Future<std::shared_ptr<BigInt>> NodeRippleLikeBlockchainExplorer::getBaseReserve() {
            return getServerInfo("reserve_base_xrp", FieldTypes::StringType);
        }

        Future<std::shared_ptr<BigInt>> NodeRippleLikeBlockchainExplorer::getLedgerSequence() {
            return getServerInfo("seq", FieldTypes::StringType);
        }

        Future<std::shared_ptr<BigInt>> NodeRippleLikeBlockchainExplorer::getServerInfo(
            const std::string &field, FieldTypes type
        ) {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("server_info");
            auto requestBody = bodyRequest.getString();
            bool parseNumberAsString = type == FieldTypes::StringType;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .json(parseNumberAsString).mapPtr<BigInt>(getContext(), [field, type](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        //Is there a result field ?
                        if (!json.IsObject() || !json.HasMember("result") ||
                            !json["result"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get base reserve from network, no (or malformed) field \"result\" in response");
                        }

                        //Is there an info field ?
                        auto resultObj = json["result"].GetObject();
                        if (!resultObj.HasMember("info") || !resultObj["info"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get base reserve from network, no (or malformed) field \"info\" in response");
                        }

                        //Is there a validated_ledger field ?
                        auto infoObj = resultObj["info"].GetObject();
                        if (!infoObj.HasMember("validated_ledger") || !infoObj["validated_ledger"].IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get base reserve from network, no (or malformed) field \"validated_ledger\" in response");
                        }

                        switch (type) {
                            case FieldTypes::StringType: {
                                //Is there a reserve_base_xrp field ?
                                auto reserveObj = infoObj["validated_ledger"].GetObject();
                                if (!reserveObj.HasMember(field.c_str()) || !reserveObj[field.c_str()].IsString()) {
                                    throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                         fmt::format("Failed to get {} from network, no (or malformed) field \"{}\" in response", field, field));
                                }

                                auto value = reserveObj[field.c_str()].GetString();
                                return std::make_shared<BigInt>(value);
                            }
                            case FieldTypes::NumberType: {
                                //Is there a reserve_base_xrp field ?
                                auto reserveObj = infoObj["validated_ledger"].GetObject();
                                if (!reserveObj.HasMember(field.c_str()) || !reserveObj[field.c_str()].IsDouble()) {
                                    throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                         fmt::format("Failed to get {} from network, no (or malformed) field \"{}\" in response <<<", field, field));
                                }

                                auto value = reserveObj[field.c_str()].GetDouble();
                                return std::make_shared<BigInt>(static_cast<unsigned int>(value * pow(10, currencies::RIPPLE.units[1].numberOfDecimal)));
                            }
                        }

                        throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Failed to get {} which has an unkown field type", field);
                    });
        }

        Future<String> NodeRippleLikeBlockchainExplorer::pushLedgerApiTransaction(
            const std::vector<uint8_t> &transaction
        ) {
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

                        if (resultObj.HasMember("engine_result") && resultObj["engine_result"] == "tesSUCCESS") {
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

        FuturePtr<RippleLikeBlockchainExplorer::TransactionsBulk> NodeRippleLikeBlockchainExplorer::getTransactions(
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
            auto requestBody = bodyRequest.getString();
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .template json<TransactionsBulk, Exception>(
                            LedgerApiParser<TransactionsBulk, RippleLikeTransactionsBulkParser>())
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

        FuturePtr<api::Block> NodeRippleLikeBlockchainExplorer::getCurrentBlock() const {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("ledger");
            bodyRequest.pushParameter("ledger_index", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .template json<api::Block, Exception>(LedgerApiParser<api::Block, RippleLikeBlockParser>())
                    .template mapPtr<api::Block>(getExplorerContext(),
                                                   [](const Either<Exception, std::shared_ptr<api::Block>> &result) {
                                                       if (result.isLeft()) {
                                                           throw result.getLeft();
                                                       } else {
                                                           return result.getRight();
                                                       }
                                                   });
        }

        FuturePtr<RippleLikeBlockchainExplorerTransaction> NodeRippleLikeBlockchainExplorer::getTransactionByHash(
            const String &transactionHash
        ) const {
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


        Future<std::shared_ptr<BigInt>> NodeRippleLikeBlockchainExplorer::getAccountInfo(
            const std::string &address,
            const std::string &key,
            const BigInt &defaultValue,
            FieldTypes type
        ) {
            NodeRippleLikeBodyRequest bodyRequest;
            bodyRequest.setMethod("account_info");
            bodyRequest.pushParameter("account", address);
            bodyRequest.pushParameter("ledger_index", std::string("validated"));
            auto requestBody = bodyRequest.getString();
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return _http->POST("", std::vector<uint8_t>(requestBody.begin(), requestBody.end()), headers)
                    .json().mapPtr<BigInt>(getContext(), [address, key, type, defaultValue](const HttpRequest::JsonResult &result) {
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
