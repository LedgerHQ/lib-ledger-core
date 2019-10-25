/*
 *
 * RpcNodeTezosLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 22/10/2019.
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


#include "RpcNodeTezosLikeBlockchainExplorer.h"
#include <api/TezosConfigurationDefaults.hpp>
#include <api/Configuration.hpp>
#include <rapidjson/document.h>
#include <api/BigInt.hpp>

namespace ledger {
    namespace core {
        RpcNodeTezosLikeBlockchainExplorer::RpcNodeTezosLikeBlockchainExplorer(
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
        RpcNodeTezosLikeBlockchainExplorer::getBalance(const std::vector<TezosLikeKeychain::Address> &addresses) {
            auto size = addresses.size();
            if (size != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get balance of 1 address from Tezos Node, but got {} addresses",
                                     addresses.size());
            }
            return getHelper(fmt::format("chains/main/blocks/head/context/contracts/{}/balance", addresses[0]->toString()),
                             "",
                             std::unordered_map<std::string, std::string>{},
                             "0"
            );
        }

        Future<std::shared_ptr<BigInt>>
        RpcNodeTezosLikeBlockchainExplorer::getFees() {
            return FuturePtr<BigInt>::successful(
                    std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_FEES)
            );
        }

        Future<String>
        RpcNodeTezosLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            std::stringstream body;
            body << '"' << hex::toString(transaction) << '"';
            auto bodyString = body.str();
            return _http->POST("injection/operation?chain=main",
                               std::vector<uint8_t>(bodyString.begin(), bodyString.end()))
                    .json().template map<String>(getExplorerContext(),
                                                 [](const HttpRequest::JsonResult &result) -> String {
                                                     auto &json = *std::get<1>(result);
                                                     if (!json.IsString()) {
                                                         throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                                              "Failed to parse broadcast transaction response, missing transaction hash");
                                                     }
                                                     return json.GetString();
                                                 });
        }

        Future<void *> RpcNodeTezosLikeBlockchainExplorer::startSession() {
            return Future<void *>::successful(new std::string());
        }

        Future<Unit> RpcNodeTezosLikeBlockchainExplorer::killSession(void *session) {
            return Future<Unit>::successful(unit);
        }

        Future<Bytes> RpcNodeTezosLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            // WARNING: not implemented
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                                 "Endpoint to get raw transactions is not implemented.");
        }

        Future<String> RpcNodeTezosLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        FuturePtr<TezosLikeBlockchainExplorer::TransactionsBulk>
        RpcNodeTezosLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                             Option<std::string> fromBlockHash,
                                                             Option<void *> session) {
            if (addresses.size() != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get transactions for 1 address from Tezos Node, but got {} addresses",
                                     addresses.size());
            }
            using EitherTransactionsBulk = Either<Exception, std::shared_ptr<TransactionsBulk>>;
            return _http->GET(fmt::format("/operations/{}", addresses[0]),
                              std::unordered_map<std::string, std::string>{},
                              "https://mystique.tzkt.io/v3/")
                    .template json<TransactionsBulk, Exception>(
                            LedgerApiParser<TransactionsBulk, TezosLikeTransactionsBulkParser>())
                    .template mapPtr<TransactionsBulk>(getExplorerContext(),
                                                       [](const EitherTransactionsBulk &result) {
                                                           if (result.isLeft()) {
                                                               // Because it fails when there are no ops
                                                               return std::make_shared<TransactionsBulk>();
                                                           } else {
                                                               return result.getRight();
                                                           }
                                                       });
        }

        FuturePtr<Block> RpcNodeTezosLikeBlockchainExplorer::getCurrentBlock() const {
            return _http->GET("chains/main/blocks/head")
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
        RpcNodeTezosLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t> RpcNodeTezosLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> RpcNodeTezosLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::TezosLikeNetworkParameters RpcNodeTezosLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string RpcNodeTezosLikeBlockchainExplorer::getExplorerVersion() const {
            return "";
        }

        Future<std::shared_ptr<BigInt>>
        RpcNodeTezosLikeBlockchainExplorer::getHelper(const std::string &url,
                                                       const std::string &field,
                                                       const std::unordered_map<std::string, std::string> &params,
                                                       const std::string &fallbackValue) {
            const bool parseNumbersAsString = true;
            auto networkId = getNetworkParameters().Identifier;

            std::string p, separator = "?";
            for (auto &param : params) {
                p += fmt::format("{}{}={}", separator, param.first, param.second);
                separator = "&";
            }

            return _http->GET(url + p,
                              std::unordered_map<std::string, std::string>())
                    .json(parseNumbersAsString)
                    .mapPtr<BigInt>(getContext(),
                                    [field, networkId, fallbackValue](const HttpRequest::JsonResult &result) {
                                        auto &json = *std::get<1>(result);
                                        if ((!json.IsObject() ||
                                             !json.HasMember(field.c_str()) ||
                                             !json[field.c_str()].IsString()) && !json.IsString()) {
                                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                                 fmt::format("Failed to get {} for {}", field,
                                                                             networkId));
                                        }
                                        std::string value = json.IsString() ? json.GetString() : json[field.c_str()].GetString();
                                        if (value == "0" && !fallbackValue.empty()) {
                                            value = fallbackValue;
                                        } else if (value.find('.') != std::string::npos) {
                                            value = api::BigInt::fromDecimalString(value, 6, ".")->toString(10);
                                        }
                                        return std::make_shared<BigInt>(value);
                                    })
                    .recover(getContext(), [fallbackValue] (const Exception &exception) {
                        return std::make_shared<BigInt>(!fallbackValue.empty() ? fallbackValue : "0");
                    });
        }

        Future<std::shared_ptr<BigInt>>
        RpcNodeTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::string &address) {
            return FuturePtr<BigInt>::successful(
                    std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_GAS_LIMIT)
            );
        }

        Future<std::shared_ptr<BigInt>>
        RpcNodeTezosLikeBlockchainExplorer::getStorage(const std::string &address) {
            return getHelper(fmt::format("chains/main/blocks/head/context/contracts/{}/storage", address),
                             "",
                             std::unordered_map<std::string, std::string>{},
                             api::TezosConfigurationDefaults::TEZOS_DEFAULT_STORAGE_LIMIT
            );
        }

        Future<std::shared_ptr<BigInt>>
        RpcNodeTezosLikeBlockchainExplorer::getCounter(const std::string &address) {
            return getHelper(fmt::format("chains/main/blocks/head/context/contracts/{}/counter", address),
                             "",
                             std::unordered_map<std::string, std::string>{},
                             "0"
            );
        }
    }
}