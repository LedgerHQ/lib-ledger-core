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
#include <wallet/tezos/explorers/api/TezosLikeTransactionsParser.h>
#include <wallet/tezos/explorers/api/TezosLikeTransactionsBulkParser.h>
#include <wallet/tezos/explorers/api/TezosLikeBlockParser.h>
#include <api/Configuration.hpp>
#include <api/ErrorCode.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>

namespace ledger {
    namespace core {

        ExternalTezosLikeBlockchainExplorer::ExternalTezosLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http,
                const api::TezosLikeNetworkParameters &parameters,
                const std::shared_ptr<ledger::core::api::DynamicObject> &configuration)
            : TezosLikeBlockchainExplorer(
                context, http, parameters, configuration,
                {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT})
        {
            _bcd = configuration->getString(api::TezosConfiguration::BCD_API)
                .value_or(api::TezosConfigurationDefaults::BCD_API_ENDPOINT);
        }


        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getBalance(const TezosLikeKeychain::Address &address) const {
            return getHelper(fmt::format("account/{}", address->toString()),
                            "total_balance",
                            std::unordered_map<std::string, std::string>{},
                            "0",
                            "",
                            true);
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getFees() const {
            // The best value is probably
            // divider_tx = (n_ops - n_ops_failed - n_ops_contract -
            //     n_seed_nonce_revelation - n_double_baking_evidence -
            //     n_double_endorsement_evidence - n_endorsement - n_reveal)
            // so we are counting all
            // n_delegation + n_origination + n_activation + n_tx
            // But the deviation of fees in all those transactions is too high so the mean fees value would not be
            // accurate anyway.
            //
            // Therefore, we only return totalFees/n_tx and leave the caller make some adjustments on the value
            // afterwards
            const bool parseNumbersAsString = true;
            return _http->GET("block/head")
                    .json(parseNumbersAsString).mapPtr<BigInt>(getContext(), [=](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);

                        //Is there a fees field ?
                        if (!json.IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                  fmt::format("Failed to get fees from network, no (or malformed) response"));
                        }

                        // Return 0 if the block had no transaction at all
                        const auto txCountField = "n_tx";
                        if (!json.HasMember(txCountField) || !json[txCountField].IsString()) {
                            return std::make_shared<BigInt>(0);
                        }
                        const auto totalTx = api::BigInt::fromIntegerString(json[txCountField].GetString(), 10);

                        auto getFieldValue = [&json](const char* fieldName) -> std::string {
                            std::string value;
                            if (json.HasMember(fieldName) && json[fieldName].IsString()) {
                                value = json[fieldName].GetString();
                            }
                            return value;
                        };
                        //try first with "fee" else with "fees"
                        std::string feesValueStr = getFieldValue("fee");
                        if (feesValueStr.empty()) {
                            feesValueStr = getFieldValue("fees");
                        }
                        if (feesValueStr.empty()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                  "Failed to get fees from network, no (or malformed) response");
                        }

                        const auto totalFees = api::BigInt::fromDecimalString(feesValueStr, 6, ".");
                        std::string fees = api::TezosConfigurationDefaults::TEZOS_DEFAULT_FEES;
                        if (fees != "0" && totalTx->intValue() != 0) {
                            fees = totalFees->divide(totalTx)->toString(10);
                        }
                        // Since nodes are giving some awkward values, we set a threshold to avoid
                        // having really fees
                        // Factor for threshold is inspired from other XTZ wallets
                        return std::make_shared<BigInt>(std::min(std::stoi(fees), std::stoi(api::TezosConfigurationDefaults::TEZOS_DEFAULT_MAX_FEES)));
                    });
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getGasPrice() const {
            const bool parseNumbersAsString = true;
            const auto gasPriceField = "gas_price";

            return _http->GET("block/head")
                    .json(parseNumbersAsString).mapPtr<BigInt>(getContext(), [=](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);

                        if (!json.IsObject() || !json.HasMember(gasPriceField) ||
                            !json[gasPriceField].IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                  fmt::format("Failed to get gas_price from network, no (or malformed) field \"{}\" in response", gasPriceField));
                        }
                        const std::string apiGasPrice = json[gasPriceField].GetString();
                        const std::string picoTezGasPrice = api::BigInt::fromDecimalString(apiGasPrice, 6, ".")->toString(10);
                        return std::make_shared<BigInt>(std::stoi(picoTezGasPrice));
                    });
        }

        Future<String>
        ExternalTezosLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            std::stringstream body;
            body << '"' << hex::toString(transaction) << '"';
            auto bodyString = body.str();
            return _http->POST("/injection/operation?chain=main",
                                std::vector<uint8_t>(bodyString.begin(), bodyString.end()),
                                std::unordered_map<std::string, std::string>{{"Content-Type", "application/json"}},
                                getRPCNodeEndpoint())
                    .json().template map<String>(getContext(),
                                                  [](const HttpRequest::JsonResult &result) -> String {
                                                      auto &json = *std::get<1>(result);

                                                      if (!json.IsString()) {
                                                          throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                                              "Failed to parse broadcast transaction response, missing transaction hash");
                                                      }
                                                      return json.GetString();
                                                  });
        }

        FuturePtr<TezosLikeBlockchainExplorer::TransactionsBulk>
        ExternalTezosLikeBlockchainExplorer::getTransactions(const std::string& address,
                                                             const Either<std::string, uint32_t>& token) const {

            const uint64_t localOffset = token.isRight() ? token.getRight() : 0;
            const uint64_t limit = 100;
            std::string params = fmt::format("?limit={}", limit);
            if (localOffset > 0) {
                params += fmt::format("&offset={}", localOffset);
            }
            using EitherTransactionsBulk = Either<Exception, std::shared_ptr<TransactionsBulk>>;
            return _http->GET(fmt::format("account/{}/op{}", address, params))
                    .template json<TransactionsBulk, Exception>(
                            LedgerApiParser<TransactionsBulk,
                            TezosLikeTransactionsBulkParser>())
                    .template mapPtr<TransactionsBulk>(getContext(),
                                                            [limit](const EitherTransactionsBulk &result) {
                                                                if (result.isLeft()) {
                                                                    // Because it fails when there are no ops
                                                                    return std::make_shared<TransactionsBulk>();
                                                                } else {
                                                                    result.getRight()->hasNext = result.getRight()->transactions.size() == limit;
                                                                    return result.getRight();
                                                                }
                                                            });
        }

        FuturePtr<Block>
        ExternalTezosLikeBlockchainExplorer::getCurrentBlock() const {
            return _http->GET("block/head")
                    .template json<Block, Exception>(LedgerApiParser<Block, TezosLikeBlockParser>())
                    .template mapPtr<Block>(getContext(),
                                            [](const Either<Exception, std::shared_ptr<Block>> &result) {
                                                if (result.isLeft()) {
                                                    throw result.getLeft();
                                                } else {
                                                    return result.getRight();
                                                }
                                            });
        }

        std::string ExternalTezosLikeBlockchainExplorer::getExplorerVersion() const {
            return "";
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::string &address) const {
            return FuturePtr<BigInt>::successful(
                    std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_GAS_LIMIT)
            );
        }

        Future<std::shared_ptr<GasLimit>>
        ExternalTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::shared_ptr<TezosLikeTransactionApi> &tx) const {
            return TezosLikeBlockchainExplorer::getEstimatedGasLimit(_http, getContext(), tx);
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getStorage(const std::string &address) const {
            return FuturePtr<BigInt>::successful(
                    std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_STORAGE_LIMIT)
            );
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getCounter(const std::string &address) const {
            return getHelper(fmt::format("/chains/main/blocks/head/context/contracts/{}/counter", address),
                              "",
                              std::unordered_map<std::string, std::string>{},
                              "0",
                              getRPCNodeEndpoint()
            );
        }

        Future<std::vector<uint8_t>>
        ExternalTezosLikeBlockchainExplorer::forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx) const {
            return TezosLikeBlockchainExplorer::forgeKTOperation(tx, getExplorerContext(), _http, getRPCNodeEndpoint());
        }

        Future<std::string>
        ExternalTezosLikeBlockchainExplorer::getManagerKey(const std::string &address) const {
            return TezosLikeBlockchainExplorer::getManagerKey(address, getExplorerContext(), _http, getRPCNodeEndpoint());
        }

        Future<bool>
        ExternalTezosLikeBlockchainExplorer::isAllocated(const std::string &address) const {
            return TezosLikeBlockchainExplorer::isAllocated(address, getExplorerContext(), _http, getRPCNodeEndpoint());
        }

        Future<std::string>
        ExternalTezosLikeBlockchainExplorer::getCurrentDelegate(const std::string &address) const {
            return TezosLikeBlockchainExplorer::getCurrentDelegate(address, getExplorerContext(), _http, getRPCNodeEndpoint());
        }

        Future<bool>
        ExternalTezosLikeBlockchainExplorer::isFunded(const std::string &address) const {
            return _http->GET(fmt::format("account/{}", address))
                .json(false, true).map<bool>(getExplorerContext(), [=](const HttpRequest::JsonResult &result) {
                    auto& connection = *std::get<0>(result);
                    if (connection.getStatusCode() == 404) {
                        //an empty account
                        return false;
                    }
                    else if (connection.getStatusCode() < 200 || connection.getStatusCode() >= 300) {
                        throw Exception(api::ErrorCode::HTTP_ERROR, connection.getStatusText());
                    }
                    else {
                        auto& json = *std::get<1>(result);

                        // look for the is_funded field
                        const auto field = "is_funded";
                        if (!json.IsObject() || !json.HasMember(field) ||
                            !json[field].IsBool()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                "Failed to get is_funded from network, no (or malformed) field \"result\" in response");
                        }

                        return json[field].GetBool();
                    }
                });
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getTokenBalance(const std::string& accountAddress,
                                                             const std::string& tokenAddress) const {
            const auto parseNumbersAsString = true;
            return _http->GET(fmt::format("/account/mainnet/{}", accountAddress), {}, _bcd)
                .json(parseNumbersAsString)
                .mapPtr<BigInt>(getContext(),
                    [=](const HttpRequest::JsonResult &result) {
                        const auto &json = *std::get<1>(result);
                        if (!json.HasMember("tokens") || !json["tokens"].IsArray()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR,
                                fmt::format("Failed to get tokens for {}, no (or malformed) field `tokens` in response", accountAddress));
                        }

                        const auto tokens = json["tokens"].GetArray();
                        for (const auto& token : tokens) {
                            if (!token.HasMember("contract") || !token["contract"].IsString()) {
                                throw make_exception(api::ErrorCode::HTTP_ERROR,
                                    "Failed to get contract from network, no (or malformed) field `contract` in response");
                            }
                            if (token["contract"].GetString() == tokenAddress) {
                                if (!token.HasMember("balance") || !token["balance"].IsString()) {
                                    throw make_exception(api::ErrorCode::HTTP_ERROR,
                                        "Failed to get contract balance from network, no (or malformed) field `balance` in response");
                                }
                                return std::make_shared<BigInt>(BigInt::fromString(token["balance"].GetString()));
                            }
                        }
                        return std::make_shared<BigInt>(BigInt::ZERO);
                    });
        }

        Future<bool>
        ExternalTezosLikeBlockchainExplorer::isDelegate(const std::string &address) const {
            return _http->GET(fmt::format("account/{}", address))
                .json(false).map<bool>(getExplorerContext(), [=](const HttpRequest::JsonResult &result) {
                    auto& json = *std::get<1>(result);
                    // look for the is_active_delegate field
                    const auto field = "is_active_delegate";
                    if (!json.IsObject() || !json.HasMember(field) ||
                        !json[field].IsBool()) {
                        throw make_exception(api::ErrorCode::HTTP_ERROR,
                                            "Failed to get is_active_delegate from network, no (or malformed) field in response");
                    }
                    return json[field].GetBool();
                });
        }

        Future<std::shared_ptr<BigInt>>
        ExternalTezosLikeBlockchainExplorer::getHelper(const std::string &url,
                                                       const std::string &field,
                                                       const std::unordered_map<std::string, std::string> &params,
                                                       const std::string &fallbackValue,
                                                       const std::string &forceUrl,
                                                       bool isDecimal) const {
            const bool parseNumbersAsString = true;
            const bool ignoreStatusCode = true;
            auto networkId = getNetworkParameters().Identifier;

            std::string p, separator = "?";
            for (auto &param : params) {
                p += fmt::format("{}{}={}", separator, param.first, param.second);
                separator = "&";
            }

            return _http->GET(url + p,
                              std::unordered_map<std::string, std::string>(),
                              forceUrl)
                    .json(parseNumbersAsString, ignoreStatusCode)
                    .mapPtr<BigInt>(getContext(),
                                    [field, networkId, fallbackValue, isDecimal](const HttpRequest::JsonResult &result) {
                                        auto& connection = *std::get<0>(result);
                                        if (connection.getStatusCode() == 404) {
                                            // it means that it’s a “logical” error (i.e. some resources not found), which
                                            // in this case we fallback to a given value
                                            return std::make_shared<BigInt>(!fallbackValue.empty() ? fallbackValue : "0");
                                        }
                                        else if (connection.getStatusCode() < 200 || connection.getStatusCode() >= 300) {
                                            throw Exception(api::ErrorCode::HTTP_ERROR, connection.getStatusText());
                                        }

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
                                        } else if (isDecimal || value.find('.') != std::string::npos) {
                                            value = api::BigInt::fromDecimalString(value, 6, ".")->toString(10);
                                        }
                                        return std::make_shared<BigInt>(value);
                                    });
        }

    } // namespace core
} // namespace ledger
