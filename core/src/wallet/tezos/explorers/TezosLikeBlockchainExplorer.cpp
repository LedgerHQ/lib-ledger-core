/*
 *
 * TezosLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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

#include "TezosLikeBlockchainExplorer.h"

#include <api/BigInt.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <unordered_map>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>

namespace ledger {
    namespace core {

        TezosLikeBlockchainExplorer::TezosLikeBlockchainExplorer(
            const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
            const std::vector<std::string> &matchableKeys) : ConfigurationMatchable(matchableKeys) {
            setConfiguration(configuration);
            _rpcNode = configuration->getString(api::TezosConfiguration::TEZOS_NODE)
                           .value_or(api::TezosConfigurationDefaults::TEZOS_DEFAULT_NODE);
        }

        Future<std::vector<uint8_t>> TezosLikeBlockchainExplorer::forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx,
                                                                                   const std::shared_ptr<api::ExecutionContext> &context,
                                                                                   const std::shared_ptr<HttpClient> &http,
                                                                                   const std::string &rpcNode) {
            std::string params;
            switch (tx->getType()) {
            case api::TezosOperationTag::OPERATION_TAG_TRANSACTION:
                params = fmt::format("\"parameters\":"
                                     "{{\"entrypoint\":\"do\",\"value\":["
                                     "{{\"prim\":\"DROP\"}},"
                                     "{{\"prim\":\"NIL\",\"args\":[{{\"prim\":\"operation\"}}]}},"
                                     "{{\"prim\":\"PUSH\",\"args\":[{{\"prim\":\"key_hash\"}},{{\"string\":\"{}\"}}]}},"
                                     "{{\"prim\":\"IMPLICIT_ACCOUNT\"}},"
                                     "{{\"prim\":\"PUSH\",\"args\":[{{\"prim\":\"mutez\"}},{{\"int\":\"{}\"}}]}},"
                                     "{{\"prim\":\"UNIT\"}},"
                                     "{{\"prim\":\"TRANSFER_TOKENS\"}},"
                                     "{{\"prim\":\"CONS\"}}]}}",
                                     tx->getReceiver()->toBase58(),
                                     tx->getValue()->toString());
                break;
            case api::TezosOperationTag::OPERATION_TAG_DELEGATION:
                params = "\"parameters\":"
                         "{{\"entrypoint\":\"do\",\"value\":["
                         "{{\"prim\":\"DROP\"}},"
                         "{{\"prim\":\"NIL\",\"args\":[{{\"prim\":\"operation\"}}]}},"
                         "{{\"prim\":\"NONE\",\"args\":[{{\"prim\":\"key_hash\"}}]}},"
                         "{{\"prim\":\"SET_DELEGATE\"}},"
                         "{{\"prim\":\"CONS\"}}]}}";
                break;
            default:
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Only supported operations from originated accounts: Transaction & (Remove) of delegation.");
            }

            // Increment counter if needs revelation
            // Arf BigInts ...
            auto counter                    = tx->toReveal() ? (BigInt(tx->getCounter()->toString(10)) + BigInt(1)).toString() : tx->getCounter()->toString(10);
            auto bodyString                 = fmt::format("{{\"branch\":\"{}\","
                                                                          "\"contents\":[{{\"kind\":\"transaction\","
                                                                          "\"fee\":\"{}\",\"gas_limit\":\"{}\",\"storage_limit\":\"{}\","
                                                                          "\"amount\":\"{}\",\"destination\":\"{}\",{},"
                                                                          "\"source\":\"{}\",\"counter\":\"{}\"}}]}}",
                                                          tx->getBlockHash().value_or(""),
                                                          tx->getFees()->toString(),
                                                          "27000",
                                                          tx->getStorageLimit()->toString(10),
                                                          0,
                                                          tx->getSender()->toBase58(),
                                                          params,
                                                          tx->getManagerAddress(),
                                                          counter);
            const bool parseNumbersAsString = true;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return http->POST("/chains/main/blocks/head/helpers/forge/operations",
                              std::vector<uint8_t>(bodyString.begin(), bodyString.end()),
                              headers,
                              rpcNode)
                .json(parseNumbersAsString)
                .map<std::vector<uint8_t>>(context, [](const HttpRequest::JsonResult &result) {
                    auto &json = *std::get<1>(result);
                    if (!json.IsString()) {
                        throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to forge operation.");
                    }
                    auto info = json.GetString();
                    return hex::toByteArray(info);
                });
        }

        Future<std::string> TezosLikeBlockchainExplorer::getManagerKey(const std::string &address,
                                                                       const std::shared_ptr<api::ExecutionContext> &context,
                                                                       const std::shared_ptr<HttpClient> &http,
                                                                       const std::string &rpcNode) {
            const bool parseNumbersAsString = true;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return http->GET(fmt::format("/chains/main/blocks/head/context/contracts/{}/manager_key", address),
                             std::unordered_map<std::string, std::string>{},
                             rpcNode)
                .json(parseNumbersAsString)
                .map<std::string>(context, [](const HttpRequest::JsonResult &result) {
                    auto &json = *std::get<1>(result);
                    if (!json.IsString()) {
                        std::cout << "getManagerKey: not a string" << std::endl;
                        // Possible if address was not revealed yet
                        return "";
                    }
                    return json.GetString();
                })
                .recover(context, [](const Exception &exception) {
                    std::cout << "getManagerKey exception" << std::endl;
                    // for KT we got an 404 instead of just a null value as we would expect
                    return "";
                });
        }

        Future<bool> TezosLikeBlockchainExplorer::isAllocated(const std::string &address,
                                                              const std::shared_ptr<api::ExecutionContext> &context,
                                                              const std::shared_ptr<HttpClient> &http,
                                                              const std::string &rpcNode) {
            const bool parseNumbersAsString = true;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return http->GET(fmt::format("/chains/main/blocks/head/context/contracts/{}", address),
                             std::unordered_map<std::string, std::string>{},
                             rpcNode)
                .json(parseNumbersAsString)
                .map<bool>(context, [](const HttpRequest::JsonResult &result) {
                    return true;
                })
                .recover(context, [](const Exception &exception) {
                    return false;
                });
        }

        Future<std::string> TezosLikeBlockchainExplorer::getCurrentDelegate(const std::string &address,
                                                                            const std::shared_ptr<api::ExecutionContext> &context,
                                                                            const std::shared_ptr<HttpClient> &http,
                                                                            const std::string &rpcNode) {
            const bool parseNumbersAsString = true;

            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return http->GET(fmt::format("/chains/main/blocks/head/context/contracts/{}/delegate", address),
                             std::unordered_map<std::string, std::string>{},
                             rpcNode)
                .json(parseNumbersAsString)
                .map<std::string>(context, [](const HttpRequest::JsonResult &result) {
                    auto &json = *std::get<1>(result);
                    if (!json.IsString()) {
                        return "";
                    }
                    return json.GetString();
                })
                .recover(context, [](const Exception &exception) {
                    // FIXME: throw exception to signal errors (i.e: network error)
                    return "";
                });
        }

        Future<std::string> TezosLikeBlockchainExplorer::getChainId(const std::shared_ptr<api::ExecutionContext> &context,
                                                                    const std::shared_ptr<HttpClient> &http) {
            const bool parseNumbersAsString = true;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return http->GET(fmt::format("/chains/main/chain_id"),
                             std::unordered_map<std::string, std::string>{},
                             getRPCNodeEndpoint())
                .json(parseNumbersAsString)
                .map<std::string>(context, [](const HttpRequest::JsonResult &result) {
                    auto &json = *std::get<1>(result);
                    if (!json.IsString()) {
                        // Possible if address was not revealed yet
                        return "";
                    }
                    return json.GetString();
                })
                .recover(context, [](const Exception &exception) {
                    return "";
                });
        }

        Future<std::shared_ptr<GasLimit>> TezosLikeBlockchainExplorer::getEstimatedGasLimit(
            const std::shared_ptr<HttpClient> &http,
            const std::shared_ptr<api::ExecutionContext> &context,
            const std::shared_ptr<TezosLikeTransactionApi> &tx) {
            return getChainId(context, http).flatMapPtr<GasLimit>(context, [=](const std::string &result) -> FuturePtr<GasLimit> {
                return getEstimatedGasLimit(http, context, tx, result);
            });
        }

        Future<std::shared_ptr<GasLimit>> TezosLikeBlockchainExplorer::getEstimatedGasLimit(
            const std::shared_ptr<HttpClient> &http,
            const std::shared_ptr<api::ExecutionContext> &context,
            const std::shared_ptr<TezosLikeTransactionApi> &tx,
            const std::string &strChainID) {
            const auto postPath =
                fmt::format("/chains/{}/blocks/head/helpers/scripts/run_operation", strChainID);
            const auto payload = tx->serializeJsonForDryRun(strChainID);

            const std::unordered_map<std::string, std::string> postHeaders{
                {"Accept", "application/json"},
                {"Content-Type", "application/json"}};

            const bool parseNumbersAsString = true;
            return http
                ->POST(
                    postPath,
                    std::vector<uint8_t>(payload.cbegin(), payload.cend()),
                    postHeaders,
                    getRPCNodeEndpoint())
                .json(parseNumbersAsString)
                .flatMapPtr<GasLimit>(
                    context,
                    [](const HttpRequest::JsonResult &result) -> FuturePtr<GasLimit> {
                        const auto &json = std::get<1>(result)->GetObject();
                        if (json.HasMember("kind")) {
                            throw make_exception(
                                api::ErrorCode::HTTP_ERROR,
                                std::make_shared<HttpRequest::JsonResult>(result),
                                "failed to simulate operation: {}",
                                json["kind"].GetString());
                        }

                        if (!json["contents"].IsArray() || !(json["contents"].Size() > 0)) {
                            throw make_exception(
                                api::ErrorCode::HTTP_ERROR,
                                std::make_shared<HttpRequest::JsonResult>(result),
                                "failed to get operation_result in simulation");
                        }

                        auto gas = std::make_shared<GasLimit>();

                        for (auto &content : json["contents"].GetArray()) {
                            if (!content.IsObject() ||
                                !content.GetObject().HasMember("kind") ||
                                !content.GetObject().HasMember("metadata") ||
                                !content.GetObject()["metadata"].HasMember("operation_result")) {
                                throw make_exception(
                                    api::ErrorCode::HTTP_ERROR,
                                    std::make_shared<HttpRequest::JsonResult>(result),
                                    "failed to get operation_result in simulation");
                            }

                            auto &operationResult = content.GetObject()["metadata"]
                                                        .GetObject()["operation_result"];

                            // Fail if operation_result is not .status == "applied"
                            if (!operationResult.HasMember("status") ||
                                operationResult["status"].GetString() != std::string("applied")) {
                                throw make_exception(
                                    api::ErrorCode::HTTP_ERROR,
                                    std::make_shared<HttpRequest::JsonResult>(result),
                                    "failed to simulate the operation on the Node");
                            }

                            const auto operationKind = content.GetObject()["kind"].GetString();
                            if (operationKind == std::string("transaction")) {
                                gas->transaction = BigInt::fromString(operationResult["consumed_gas"].GetString());
                            } else if (operationKind == std::string("reveal")) {
                                gas->reveal = BigInt::fromString(operationResult["consumed_gas"].GetString());
                            }
                        }

                        if (!gas->transaction.isZero()) {
                            return FuturePtr<GasLimit>::successful(gas);
                        } else {
                            throw make_exception(
                                api::ErrorCode::HTTP_ERROR,
                                std::make_shared<HttpRequest::JsonResult>(result),
                                "failed to get transaction kind in simulation");
                        }
                    })
                .recover(context, [](const Exception &exception) {
                    auto ecode = exception.getErrorCode();
                    // Tezos RPC returns a 500 when the transaction is not valid (bad counter, no
                    // balance, etc.) so we rethrow the tezos node error for easier debugging
                    auto body  = std::static_pointer_cast<HttpRequest::JsonResult>(
                        exception.getUserData().getValue());
                    const auto &json = *std::get<1>(*body);
                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    json.Accept(writer);
                    throw make_exception(
                        api::ErrorCode::HTTP_ERROR,
                        "failed to simulate operation: {}",
                        buffer.GetString());

                    // lambda is [noreturn], this is just left so type deduction succeeds and
                    // compiles
                    return std::make_shared<GasLimit>();
                });
        }

    } // namespace core
} // namespace ledger
