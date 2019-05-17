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
            return _http->GET("head")
                    .json().mapPtr<BigInt>(getContext(), [](const HttpRequest::JsonResult &result) {
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
            using EitherTransactionsBulk = Either<Exception, std::shared_ptr<TransactionsBulk>>;
            auto self = shared_from_this();
            std::vector<std::string> txTypes {"Transaction", "Reveal", "Origination"};
            static std::function<FuturePtr<TransactionsBulk> (const std::shared_ptr<TransactionsBulk> &txsBulk, size_t type)> getTransactionsOfType = [=] (const std::shared_ptr<TransactionsBulk> &txsBulk, size_t type) -> FuturePtr<TransactionsBulk> {
                return self->_http->GET(fmt::format("operations/{}?type={}", addresses[0], txTypes[type]))
                        .template json<TransactionsBulk, Exception>(LedgerApiParser<TransactionsBulk, TezosLikeTransactionsBulkParser>())
                        .template flatMapPtr<TransactionsBulk>(self->getExplorerContext(), [=](const EitherTransactionsBulk &result) {
                            if (result.isLeft()) {
                                throw result.getLeft();
                            } else {
                                txsBulk->transactions.insert(txsBulk->transactions.end(), result.getRight()->transactions.begin(), result.getRight()->transactions.end());
                                if (type < txTypes.size() - 1) {
                                    return getTransactionsOfType(txsBulk, type + 1);
                                }
                                return FuturePtr<TransactionsBulk>::successful(txsBulk);
                            }
                        });
            };
            auto transactionsBulk = std::make_shared<TransactionsBulk>();
            return getTransactionsOfType(transactionsBulk, 0);
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

        Future<std::shared_ptr<BigInt>> NodeTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::string &address) {
            // TODO: replace with final call
            // return getHelper(fmt::format("/contract/{}/estimate-gas-limit", getExplorerVersion(), getNetworkParameters().Identifier, address), "estimated_gas_limit");
            return Future<std::shared_ptr<BigInt>>::successful(std::make_shared<BigInt>("10300"));
        }

        Future<std::shared_ptr<BigInt>> NodeTezosLikeBlockchainExplorer::getStorage(const std::string &address) {
            // TODO: replace with final call
            // return getHelper(fmt::format("/contract/{}/storage", getExplorerVersion(), getNetworkParameters().Identifier, address), "storage");
            return Future<std::shared_ptr<BigInt>>::successful(std::make_shared<BigInt>("300"));
        }

        Future<std::shared_ptr<BigInt>> NodeTezosLikeBlockchainExplorer::getCounter(const std::string &address) {
            // TODO: replace with final call
            return Future<std::shared_ptr<BigInt>>::successful(std::make_shared<BigInt>("0"));
            /*
            bool parseNumbersAsString = true;
            return _http->GET(fmt::format("/contract/{}/counter", address))
                    .json(parseNumbersAsString).mapPtr<BigInt>(getContext(), [address] (const HttpRequest::JsonResult& result) {
                        auto& json = *std::get<1>(result);
                        if (!json.IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get counter for {}", address);
                        }
                        return std::make_shared<BigInt>(json.GetString());
                    });
            */
        }
    }
}