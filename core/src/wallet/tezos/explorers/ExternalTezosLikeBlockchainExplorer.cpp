/*
 *
 * ExternalTezosLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 20/10/2019.
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


#include "ExternalTezosLikeBlockchainExplorer.h"
#include <api/TezosConfigurationDefaults.hpp>
#include <api/Configuration.hpp>
#include <rapidjson/document.h>
#include <api/BigInt.hpp>

namespace ledger {
    namespace core {
        ExternalTezosLikeBlockchainExplorer::ExternalTezosLikeBlockchainExplorer(
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
        ExternalTezosLikeBlockchainExplorer::getBalance(const std::vector<TezosLikeKeychain::Address> &addresses) {
            auto size = addresses.size();
            if (size != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get balance of 1 address from Tezos Node, but got {} addresses",
                                     addresses.size());
            }
            std::string addressesStr = addresses[0]->toString();
            return getHelper(fmt::format("account/{}", addressesStr),
                             "total_balance",
                             std::unordered_map<std::string, std::string>{}
            );
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getFees() {
            bool parseNumbersAsString = true;
            return _http->GET("block/head")
                    .json(parseNumbersAsString).mapPtr<BigInt>(getContext(), [](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        //Is there a fees field ?
                        if (!json.IsObject() || !json.HasMember("fees") ||
                            !json["fees"].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                 "Failed to get fees from network, no (or malformed) field \"result\" in response");
                        }
                        std::string fees = json["fees"].GetString();
                        // Sometimes network is sending 0 for fees
                        if (fees == "0") {
                            fees = api::TezosConfigurationDefaults::TEZOS_DEFAULT_FEES;
                        }
                        return std::make_shared<BigInt>(fees);
                    });
        }

        Future<String>
        ExternalTezosLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            std::stringstream body;
            body << '"' << hex::toString(transaction) << '"';
            auto bodyString = body.str();
            return _http->POST("/injection/operation?chain=main",
                               std::vector<uint8_t>(bodyString.begin(), bodyString.end()),
                               std::unordered_map<std::string, std::string>{},
                               "https://mainnet.tezrpc.me")
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

        Future<void *> ExternalTezosLikeBlockchainExplorer::startSession() {
            std::string sessionToken = fmt::format("{}", std::rand());
            _sessions.insert(std::make_pair(sessionToken, 0));
            return Future<void *>::successful(new std::string(sessionToken));
        }

        Future<Unit> ExternalTezosLikeBlockchainExplorer::killSession(void *session) {
            if (session) {
                _sessions.erase(*((std::string *)session));
            }
            return Future<Unit>::successful(unit);
        }

        Future<Bytes> ExternalTezosLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            // WARNING: not implemented
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                                 "Endpoint to get raw transactions is not implemented.");
        }

        Future<String> ExternalTezosLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        FuturePtr<TezosLikeBlockchainExplorer::TransactionsBulk>
        ExternalTezosLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                             Option<std::string> fromBlockHash,
                                                             Option<void *> session) {
            uint64_t offset = 0, limit = 100;
            if (session.hasValue()) {
                auto s = _sessions[*((std::string *)session.getValue())];
                offset = limit * s;
                _sessions[*((std::string *)session.getValue())]++;
            }
            if (addresses.size() != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get transactions for 1 address from Tezos Node, but got {} addresses",
                                     addresses.size());
            }
            std::string params = fmt::format("?limit={}",limit);
            if (offset > 0) {
                params += fmt::format("?offset={}",offset);
            }
            using EitherTransactionsBulk = Either<Exception, std::shared_ptr<TransactionsBulk>>;
            return _http->GET(fmt::format("account/{}/op", addresses[0]))
                    .template json<TransactionsBulk, Exception>(
                            LedgerApiParser<TransactionsBulk,
                            TezosLikeTransactionsBulkParser>())
                    .template mapPtr<TransactionsBulk>(getExplorerContext(),
                                                           [=](const EitherTransactionsBulk &result) {
                                                               if (result.isLeft()) {
                                                                   throw result.getLeft();
                                                               } else {
                                                                   return result.getRight();
                                                               }
                                                           });
        }

        FuturePtr<Block> ExternalTezosLikeBlockchainExplorer::getCurrentBlock() const {
            return _http->GET("block/head")
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
        ExternalTezosLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t> ExternalTezosLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> ExternalTezosLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::TezosLikeNetworkParameters ExternalTezosLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string ExternalTezosLikeBlockchainExplorer::getExplorerVersion() const {
            return "";
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getHelper(const std::string &url,
                                                       const std::string &field,
                                                       const std::unordered_map<std::string, std::string> &params,
                                                       const std::string &fallbackValue) {
            bool parseNumbersAsString = true;
            auto networkId = getNetworkParameters().Identifier;

            std::string p, separator = "?";
            for (auto &param : params) {
                p += fmt::format("{}{}={}", separator, param.first, param.second);
                separator = "&";
            }

            return _http->GET(url + p, std::unordered_map<std::string, std::string>())
                    .json(parseNumbersAsString)
                    .mapPtr<BigInt>(getContext(),
                                    [field, networkId, fallbackValue](const HttpRequest::JsonResult &result) {
                                        auto &json = *std::get<1>(result);
                                        if (!json.IsObject() ||
                                                !json.HasMember(field.c_str()) ||
                                                !json[field.c_str()].IsString()) {
                                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                                 fmt::format("Failed to get {} for {}", field,
                                                                             networkId));
                                        }
                                        std::string value = json[field.c_str()].GetString();
                                        if (value == "0" && !fallbackValue.empty()) {
                                            value = fallbackValue;
                                        } else if (value.find('.') != std::string::npos) {
                                            value = api::BigInt::fromDecimalString(value, 6, ".")->toString(10);
                                        }
                                        return std::make_shared<BigInt>(value);
                                    });
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::string &address) {
            return FuturePtr<BigInt>::successful(
                    std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_GAS_LIMIT)
            );
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getStorage(const std::string &address) {
            return FuturePtr<BigInt>::successful(
                    std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_STORAGE_LIMIT)
            );
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getCounter(const std::string &address) {
            return getHelper(fmt::format("account/{}", address),
                             "n_tx",
                             std::unordered_map<std::string, std::string>{}
            );
        }
    }
}