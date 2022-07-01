/*
 *
 * BakingBadTezosLikeBlockchainExplorer
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

#include "BakingBadTezosLikeBlockchainExplorer.h"

#include "wallet/currencies.hpp"

#include <api/Configuration.hpp>
#include <api/ErrorCode.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <clocale>
#include <sstream>

namespace ledger {
    namespace core {
        namespace api {
            void from_json(const nlohmann::json &j, TezosOperationTag &t) {
                auto value = j.get<std::string>();
                static std::unordered_map<std::string, api::TezosOperationTag> opTags{
                    std::make_pair("reveal", api::TezosOperationTag::OPERATION_TAG_REVEAL),
                    std::make_pair("transaction", api::TezosOperationTag::OPERATION_TAG_TRANSACTION),
                    std::make_pair("origination", api::TezosOperationTag::OPERATION_TAG_ORIGINATION),
                    std::make_pair("delegation", api::TezosOperationTag::OPERATION_TAG_DELEGATION),
                };

                if (opTags.count(value)) {
                    t = opTags[value];
                } else {
                    t = api::TezosOperationTag::OPERATION_TAG_NONE;
                }
            }
        } // namespace api

        void from_json(const nlohmann::json &j, BigInt &i) {
            if (j.is_number_integer()) {
                i = ledger::core::BigInt::fromScalar(j.get<int>());
            } else {
                i = ledger::core::BigInt::fromString(j.get<std::string>());
            }
        }

        void from_json(const nlohmann::json &j, TezosLikeBlockchainExplorer::Transaction &t) {
            j.at("hash").get_to(t.hash);
            auto timestamp  = j.at("timestamp").get<std::string>();
            t.receivedAt    = DateUtils::fromJSON(timestamp);
            t.fees          = BigInt(j.value("bakerFee", 0) + j.value("storageFee", 0) + j.value("allocationFee", 0));
            t.storage_limit = BigInt(j.value("storageLimit", 0));
            j.at("gasLimit").get_to(t.gas_limit);
            j.at("sender").at("address").get_to(t.sender);
            TezosLikeBlockchainExplorer::Block block;
            j.at("block").get_to(block.hash);
            j.at("level").get_to(block.height);
            block.currencyName = currencies::TEZOS.name;
            block.time         = t.receivedAt;
            t.block            = block;
            j.at("type").get_to(t.type);
            t.status = static_cast<uint64_t>(j.at("status").get<std::string>() == "applied");
            j.at("counter").get_to(t.counter);

            // Missing 'confirmation' data in api, but it doesn't seem quite important for WD as
            // it is computed in co.ledger.walllet.daemon.models.Operations.scala::getView()
            // but it might be an issue for lama if it relies on the "confirmation" field of the DB ?
            t.confirmations = 0;

            switch (t.type) {
            case api::TezosOperationTag::OPERATION_TAG_REVEAL:
                // TODO: missing info ?
                // j.at("hash").get_to(t.publicKey);
                break;
            case api::TezosOperationTag::OPERATION_TAG_TRANSACTION:
                j.at("amount").get_to(t.value);
                j.at("target").at("address").get_to(t.receiver);
                break;
            case api::TezosOperationTag::OPERATION_TAG_ORIGINATION:
                t.originatedAccount = TezosLikeBlockchainExplorerOriginatedAccount();
                j.at("originatedContract").at("address").get_to(t.originatedAccount->address);
                break;
            case api::TezosOperationTag::OPERATION_TAG_DELEGATION:
                j.at("amount").get_to(t.value);
                break;
            default: break;
            }
        }

        struct TzKTParser {
            void from_json(const nlohmann::json &j, TezosLikeBlockchainExplorer::TransactionsBulk &b) {
                j.get_to(b.transactions);
            }

            void from_json(const nlohmann::json &j, BigInt &b) {
                j.get_to(b);
            }

            void from_json(const nlohmann::json &j, Block &b) {
                b.currencyName = currencies::TEZOS.name;
                j[0].at("hash").get_to(b.hash);
                j[0].at("level").get_to(b.height);
                std::string timestamp = j[0].at("timestamp").get<std::string>();
                b.time                = DateUtils::fromJSON(timestamp);
            }
        };

        BakingBadTezosLikeBlockchainExplorer::BakingBadTezosLikeBlockchainExplorer(
            const std::shared_ptr<api::ExecutionContext> &context,
            const std::shared_ptr<HttpClient> &http,
            const api::TezosLikeNetworkParameters &parameters,
            const std::shared_ptr<api::DynamicObject> &configuration) : DedicatedContext(context),
                                                                        TezosLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http       = http;
            _parameters = parameters;
            _bcd        = configuration->getString(api::TezosConfiguration::BCD_API)
                       .value_or(api::TezosConfigurationDefaults::BCD_API_ENDPOINT);
        }

        Future<std::shared_ptr<BigInt>>
        BakingBadTezosLikeBlockchainExplorer::getBalance(const std::vector<TezosLikeKeychain::Address> &addresses) {
            auto size = addresses.size();
            if (size != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get balance of 1 address from Tezos Node, but got {} addresses",
                                     addresses.size());
            }
            std::string addressesStr = addresses[0]->toString();

            return _http->GET(fmt::format("v1/accounts/{}/balance", addressesStr))
                .template json<BigInt, TzKTParser, Exception>()
                .template mapPtr<BigInt>(getExplorerContext(),
                                         [](const Either<Exception, std::shared_ptr<BigInt>> &result) {
                                             if (result.isLeft()) {
                                                 throw result.getLeft();
                                             } else {
                                                 return result.getRight();
                                             }
                                         });
        }

        Future<std::shared_ptr<BigInt>>
        BakingBadTezosLikeBlockchainExplorer::getFees() {
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
                .json(parseNumbersAsString)
                .mapPtr<BigInt>(getContext(), [=](const HttpRequest::JsonResult &result) {
                    auto &json = *std::get<1>(result);

                    // Is there a fees field ?
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

                    auto getFieldValue = [&json](const char *fieldName) -> std::string {
                        std::string value;
                        if (json.HasMember(fieldName) && json[fieldName].IsString()) {
                            value = json[fieldName].GetString();
                        }
                        return value;
                    };
                    // try first with "fee" else with "fees"
                    std::string feesValueStr = getFieldValue("fee");
                    if (feesValueStr.empty()) {
                        feesValueStr = getFieldValue("fees");
                    }
                    if (feesValueStr.empty()) {
                        throw make_exception(api::ErrorCode::HTTP_ERROR,
                                             "Failed to get fees from network, no (or malformed) response");
                    }

                    const auto totalFees = api::BigInt::fromDecimalString(feesValueStr, 6, ".");
                    std::string fees     = api::TezosConfigurationDefaults::TEZOS_DEFAULT_FEES;
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
        BakingBadTezosLikeBlockchainExplorer::getGasPrice() {
            const bool parseNumbersAsString = true;
            // Since tzindex 12.01, we don't have gas_price field anymore
            // We have to calculate it instead from gas_used and fee
            const auto gasUsedField         = "gas_used";
            const auto feeField             = "fee";
            // We still use the legacy field in case we have a rollback
            const auto gasPriceField        = "gas_price";

            return _http->GET("block/head")
                .json(parseNumbersAsString)
                .mapPtr<BigInt>(getContext(), [=](const HttpRequest::JsonResult &result) {
                    // Fix locale issue in conversion string from/to double
                    struct ScopedLocale {
                        ScopedLocale() {
                            _locale = std::setlocale(LC_NUMERIC, nullptr);
                            std::setlocale(LC_NUMERIC, "C");
                        }
                        ~ScopedLocale() {
                            std::setlocale(LC_NUMERIC, _locale.c_str());
                        }

                      private:
                        std::string _locale;
                    } _;

                    auto &json = *std::get<1>(result);

                    if (!json.IsObject()) {
                        throw make_exception(
                            api::ErrorCode::HTTP_ERROR, "Failed to compute gas_price from network, block/head is not a JSON object");
                    }
                    std::string gasPrice;
                    // Legacy field access
                    if (json.HasMember(gasPriceField) && json[gasPriceField].IsString()) {
                        gasPrice = json[gasPriceField].GetString();
                    }
                    // OR
                    // tzindex v12+ gas_price compute
                    else {
                        if (!json.HasMember(gasUsedField) || !json[gasUsedField].IsString()) {
                            throw make_exception(
                                api::ErrorCode::HTTP_ERROR, fmt::format(
                                                                "Failed to compute gas_price from network, no (or malformed) field \"{}\" in response",
                                                                gasUsedField));
                        }
                        if (!json.HasMember(feeField) || !json[feeField].IsString()) {
                            throw make_exception(
                                api::ErrorCode::HTTP_ERROR, fmt::format(
                                                                "Failed to compute gas_price from network, no (or malformed) field \"{}\" in response",
                                                                feeField));
                        }
                        const uint64_t apiGasUsed = std::stoull(json[gasUsedField].GetString());
                        if (apiGasUsed == 0) {
                            throw make_exception(
                                api::ErrorCode::HTTP_ERROR, "Failed to compute gas_price from network, gas_used of HEAD block is 0");
                        }

                        const auto &feeFieldStringValue = json[feeField].GetString();
                        double apiFee                   = 0.;
                        try {
                            apiFee = std::stod(feeFieldStringValue);
                        } catch (const std::invalid_argument &e) {
                            throw make_exception(
                                api::ErrorCode::INVALID_ARGUMENT, fmt::format(
                                                                      "Failed to compute gas_price from network, issue converting from string with fee of HEAD block equal to \"{}\": {}",
                                                                      feeFieldStringValue,
                                                                      e.what()));
                        } catch (const std::out_of_range &e) {
                            throw make_exception(
                                api::ErrorCode::OUT_OF_RANGE, fmt::format(
                                                                  "Failed to compute gas_price from network, issue casting to double with fee of HEAD block equal to \"{}\" : {}",
                                                                  feeFieldStringValue,
                                                                  e.what()));
                        }

                        const double numericGasPrice = apiFee / static_cast<double>(apiGasUsed);
                        std::ostringstream ss;
                        ss.precision(std::numeric_limits<double>::digits10);
                        ss << std::fixed << numericGasPrice;
                        gasPrice = ss.str();
                    }

                    const std::string picoTezGasPrice = api::BigInt::fromDecimalString(gasPrice, 6, ".")->toString(10);
                    int intPicoTezGasPrice            = 0;
                    try {
                        intPicoTezGasPrice = std::stoi(picoTezGasPrice);
                    } catch (const std::invalid_argument &e) {
                        throw make_exception(
                            api::ErrorCode::INVALID_ARGUMENT, fmt::format(
                                                                  "Failed to compute gas_price from network, issue converting from string with picoTezGasPrice equal to \"{}\": {}",
                                                                  picoTezGasPrice,
                                                                  e.what()));
                    } catch (const std::out_of_range &e) {
                        throw make_exception(
                            api::ErrorCode::OUT_OF_RANGE, fmt::format(
                                                              "Failed to compute gas_price from network, issue casting to int with picoTezGasPrice equal to \"{}\" : {}",
                                                              picoTezGasPrice,
                                                              e.what()));
                    }

                    return std::make_shared<BigInt>(intPicoTezGasPrice);
                });
        }

        Future<String>
        BakingBadTezosLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction, const std::string &correlationId) {
            std::stringstream body;
            body << '"' << hex::toString(transaction) << '"';
            auto bodyString = body.str();
            return _http->POST("/injection/operation?chain=main",
                               std::vector<uint8_t>(bodyString.begin(), bodyString.end()),
                               std::unordered_map<std::string, std::string>{{"Content-Type", "application/json"}},
                               getRPCNodeEndpoint())
                .json()
                .template map<String>(getExplorerContext(),
                                      [](const HttpRequest::JsonResult &result) -> String {
                                          auto &json = *std::get<1>(result);

                                          if (!json.IsString()) {
                                              throw make_exception(api::ErrorCode::HTTP_ERROR,
                                                                   "Failed to parse broadcast transaction response, missing transaction hash");
                                          }
                                          return json.GetString();
                                      });
        }

        Future<void *> BakingBadTezosLikeBlockchainExplorer::startSession() {
            std::string sessionToken = fmt::format("{}", std::rand());
            _sessions.insert(std::make_pair(sessionToken, 0));
            return Future<void *>::successful(new std::string(sessionToken));
        }

        Future<Unit> BakingBadTezosLikeBlockchainExplorer::killSession(void *session) {
            if (session) {
                _sessions.erase(*(reinterpret_cast<std::string *>(session)));
            }
            return Future<Unit>::successful(unit);
        }

        Future<Bytes> BakingBadTezosLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            // WARNING: not implemented
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING,
                                 "Endpoint to get raw transactions is not implemented.");
        }

        Future<String> BakingBadTezosLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction, const std::string &correlationId) {
            return pushLedgerApiTransaction(transaction, correlationId);
        }

        FuturePtr<TezosLikeBlockchainExplorer::TransactionsBulk>
        BakingBadTezosLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                              Option<std::string> offset,
                                                              Option<void *> session) {
            auto tryOffset       = Try<uint64_t>::from([=]() -> uint64_t {
                return std::stoul(offset.getValueOr(""), nullptr, 10);
            });

            uint64_t localOffset = tryOffset.isSuccess() ? tryOffset.getValue() : 0;
            uint64_t limit       = 100;
            if (session.hasValue()) {
                auto s = _sessions[*((std::string *)session.getValue())];
                localOffset += limit * s;
                _sessions[*reinterpret_cast<std::string *>(session.getValue())]++;
            }
            if (addresses.size() != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Can only get transactions for 1 address from Tezos Node, but got {} addresses",
                                     addresses.size());
            }
            std::string params = fmt::format("?limit={}", limit);
            if (localOffset > 0) {
                params += fmt::format("&lastId={}", localOffset);
            }
            using EitherTransactionsBulk = Either<Exception, std::shared_ptr<TransactionsBulk>>;
            return _http->GET(fmt::format("v1/accounts/{}/operations{}", addresses[0], params))
                .template json<TezosLikeBlockchainExplorer::TransactionsBulk, TzKTParser, Exception>()
                .template mapPtr<TransactionsBulk>(getExplorerContext(),
                                                   [limit](const EitherTransactionsBulk &result) {
                                                       if (result.isLeft()) {
                                                           throw result.getLeft();
                                                       } else {
                                                           result.getRight()->hasNext = result.getRight()->transactions.size() == limit;
                                                           return result.getRight();
                                                       }
                                                   });
        }

        FuturePtr<Block> BakingBadTezosLikeBlockchainExplorer::getCurrentBlock() const {
            return _http->GET("v1/blocks?sort.desc=id&limit=1")
                .template json<Block, TzKTParser, Exception>()
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
        BakingBadTezosLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t> BakingBadTezosLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> BakingBadTezosLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::TezosLikeNetworkParameters BakingBadTezosLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string BakingBadTezosLikeBlockchainExplorer::getExplorerVersion() const {
            return "";
        }

        Future<std::shared_ptr<BigInt>>
        BakingBadTezosLikeBlockchainExplorer::getHelper(const std::string &url,
                                                        const std::string &field,
                                                        const std::unordered_map<std::string, std::string> &params,
                                                        const std::string &fallbackValue,
                                                        const std::string &forceUrl,
                                                        bool isDecimal) {
            const bool parseNumbersAsString = true;
            const bool ignoreStatusCode     = true;
            auto networkId                  = getNetworkParameters().Identifier;

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
                                    auto &connection = *std::get<0>(result);
                                    if (connection.getStatusCode() == 404) {
                                        // it means that it’s a “logical” error (i.e. some resources not found), which
                                        // in this case we fallback to a given value
                                        return std::make_shared<BigInt>(!fallbackValue.empty() ? fallbackValue : "0");
                                    } else if (connection.getStatusCode() < 200 || connection.getStatusCode() >= 300) {
                                        throw Exception(api::ErrorCode::HTTP_ERROR, connection.getStatusText());
                                    }

                                    auto &json = *std::get<1>(result);
                                    if ((!json.IsObject() ||
                                         !json.HasMember(field.c_str()) ||
                                         !json[field.c_str()].IsString()) &&
                                        !json.IsString()) {
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

        Future<std::shared_ptr<BigInt>>
        BakingBadTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::string &address) {
            return FuturePtr<BigInt>::successful(
                std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_GAS_LIMIT));
        }

        Future<std::shared_ptr<GasLimit>>
        BakingBadTezosLikeBlockchainExplorer::getEstimatedGasLimit(const std::shared_ptr<TezosLikeTransactionApi> &tx) {
            return TezosLikeBlockchainExplorer::getEstimatedGasLimit(_http, getContext(), tx);
        }

        Future<std::shared_ptr<BigInt>>
        BakingBadTezosLikeBlockchainExplorer::getStorage(const std::string &address) {
            return FuturePtr<BigInt>::successful(
                std::make_shared<BigInt>(api::TezosConfigurationDefaults::TEZOS_DEFAULT_STORAGE_LIMIT));
        }

        Future<std::shared_ptr<BigInt>>
        BakingBadTezosLikeBlockchainExplorer::getCounter(const std::string &address) {
            return getHelper(fmt::format("/chains/main/blocks/head/context/contracts/{}/counter", address),
                             "",
                             std::unordered_map<std::string, std::string>{},
                             "0",
                             getRPCNodeEndpoint());
        }

        Future<std::vector<uint8_t>> BakingBadTezosLikeBlockchainExplorer::forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx) {
            return TezosLikeBlockchainExplorer::forgeKTOperation(tx,
                                                                 getExplorerContext(),
                                                                 _http,
                                                                 getRPCNodeEndpoint());
        }

        Future<std::string> BakingBadTezosLikeBlockchainExplorer::getManagerKey(const std::string &address) {
            return TezosLikeBlockchainExplorer::getManagerKey(address,
                                                              getExplorerContext(),
                                                              _http,
                                                              getRPCNodeEndpoint());
        }

        Future<bool> BakingBadTezosLikeBlockchainExplorer::isAllocated(const std::string &address) {
            return TezosLikeBlockchainExplorer::isAllocated(address,
                                                            getExplorerContext(),
                                                            _http,
                                                            getRPCNodeEndpoint());
        }

        Future<std::string> BakingBadTezosLikeBlockchainExplorer::getCurrentDelegate(const std::string &address) {
            return TezosLikeBlockchainExplorer::getCurrentDelegate(address,
                                                                   getExplorerContext(),
                                                                   _http,
                                                                   getRPCNodeEndpoint());
        }

        Future<std::shared_ptr<BigInt>>
        BakingBadTezosLikeBlockchainExplorer::getTokenBalance(const std::string &accountAddress,
                                                              const std::string &tokenAddress) const {
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
                                    for (const auto &token : tokens) {
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

        Future<bool> BakingBadTezosLikeBlockchainExplorer::isFunded(const std::string &address) {
            auto tzaddress = TezosLikeAddress::fromBase58(address, currencies::TEZOS);
            return getBalance(std::vector<std::shared_ptr<TezosLikeAddress>>{tzaddress})
                .map<bool>(getExplorerContext(), [](const std::shared_ptr<BigInt> &balance) {
                    return balance->toInt64() != 0;
                });
        }

        Future<bool> BakingBadTezosLikeBlockchainExplorer::isDelegate(const std::string &address) {
            return _http->GET(fmt::format("account/{}", address))
                .json(false)
                .map<bool>(getExplorerContext(), [=](const HttpRequest::JsonResult &result) {
                    auto &json       = *std::get<1>(result);
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
    } // namespace core
} // namespace ledger
