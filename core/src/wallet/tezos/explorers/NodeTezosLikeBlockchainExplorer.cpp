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
            bool parseNumbersAsString = true;
            std::string addressesStr = addresses[0]->toBase58();
            return _http->GET(fmt::format("balance/{}", addressesStr))
                    .json(parseNumbersAsString).mapPtr<BigInt>(getContext(), [addressesStr](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        if (!json.IsArray() && json.Size() == 1 && json[0].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get balance for {}", addressesStr);
                        }
                        auto info = json[0].GetString();
                        return std::make_shared<BigInt>(info);
                    });
        }

        Future<std::shared_ptr<BigInt>>
        NodeTezosLikeBlockchainExplorer::getFees() {
            bool parseNumbersAsString = true;
            return _http->GET("head")
                    .json(parseNumbersAsString).mapPtr<BigInt>(getContext(), [](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        //Is there a fees field ?
                        if (!json.IsObject() || !json.HasMember("fees") ||
                            !json["fees"].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get fees from network, no (or malformed) field \"result\" in response");
                        }
                        auto fees = json["fees"].GetString();
                        return std::make_shared<BigInt>(fees);
                    });
        }

        Future<String>
        NodeTezosLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            //injection/operation
            return _http->GET("")
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
            return _http->GET("")
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
            std::string params;
            if (fromBlockHash.hasValue()) {
                params = "&block_hash=" + fromBlockHash.getValue();
            }
            auto self = shared_from_this();
            using EitherTransactionsBulk = Either<Exception, std::shared_ptr<TransactionsBulk>>;
            static std::vector<std::string> txTypes {"Transaction", "Reveal", "Origination", "Delegation"};
            // Note: we should get rid of this if we tweak explorer
            // to have all type of operations at once
            static std::function<FuturePtr<TransactionsBulk> (const std::string address,
                                                              const std::string &params,
                                                              const std::shared_ptr<TransactionsBulk> &txsBulk,
                                                              size_t type)> getTransactionsOfType = [self] (const std::string &address,
                                                                                                            const std::string &parameters,
                                                                                                            const std::shared_ptr<TransactionsBulk> &txsBulk,
                                                                                                            size_t type) -> FuturePtr<TransactionsBulk> {
                return self->_http->GET(fmt::format("operations/{}?type={}{}", address, txTypes[type], parameters))
                        .template json<TransactionsBulk, Exception>(LedgerApiParser<TransactionsBulk, TezosLikeTransactionsBulkParser>())
                        .template flatMapPtr<TransactionsBulk>(self->getExplorerContext(), [=](const EitherTransactionsBulk &result) {
                            if (result.isLeft()) {
                                throw result.getLeft();
                            } else {
                                txsBulk->transactions.insert(txsBulk->transactions.end(), result.getRight()->transactions.begin(), result.getRight()->transactions.end());
                                // Only originated accounts can delegate
                                auto isOriginated = address.find("KT") == 0;
                                if (type == txTypes.size() - 1 || (type == txTypes.size() - 2 && !isOriginated)) {
                                    return FuturePtr<TransactionsBulk>::successful(txsBulk);
                                }
                                return getTransactionsOfType(address, parameters, txsBulk, type + 1);
                            }
                        });
            };
            auto transactionsBulk = std::make_shared<TransactionsBulk>();
            return getTransactionsOfType(addresses[0], params, transactionsBulk, 0);
        }

        FuturePtr<Block> NodeTezosLikeBlockchainExplorer::getCurrentBlock() const {
            return _http->GET("head")
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

        Future<std::shared_ptr<BigInt>> NodeTezosLikeBlockchainExplorer::getHelper(const std::string &url,
                                                                                   const std::string &field,
                                                                                   const std::unordered_map<std::string, std::string> &params) {
            bool parseNumbersAsString = true;
            auto networkId = getNetworkParameters().Identifier;

            std::string p, separator = "?";
            for (auto &param : params) {
                p += fmt::format("{}{}={}", separator, param.first, param.second);
                separator = "&";
            }

            return _http->GET(url + p, std::unordered_map<std::string, std::string>()).json(parseNumbersAsString).mapPtr<BigInt>(getContext(), [field, networkId] (const HttpRequest::JsonResult& result) {
                auto& json = *std::get<1>(result);
                if (!json.IsArray() || json.Size() == 0 || json[0].IsString()) {
                    throw make_exception(api::ErrorCode::HTTP_ERROR, fmt::format("Failed to get {} for {}", field, networkId));
                }
                return std::make_shared<BigInt>(json[0].GetString());
            });
        }

        Future<std::shared_ptr<BigInt>> NodeTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::string &address) {
            return getHelper(fmt::format("/{}/estimate_gas", getExplorerVersion()), "estimated_gas_limit", std::unordered_map<std::string, std::string>{{"token", address}});
        }

        Future<std::shared_ptr<BigInt>> NodeTezosLikeBlockchainExplorer::getStorage(const std::string &address) {
            return getHelper(fmt::format("/{}/estimate_storage", getExplorerVersion()), "storage", std::unordered_map<std::string, std::string>{{"token", address}});
        }

        Future<std::shared_ptr<BigInt>> NodeTezosLikeBlockchainExplorer::getCounter(const std::string &address) {
            return getHelper(fmt::format("/{}/counter", getExplorerVersion()), "counter", std::unordered_map<std::string, std::string>{{"token", address}});
        }
    }
}