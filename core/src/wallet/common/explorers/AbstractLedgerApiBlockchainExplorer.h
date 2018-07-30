/*
 *
 * AbstractLedgerApiBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 29/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#ifndef LEDGER_CORE_ABSTRACTLEDGERAPIBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_ABSTRACTLEDGERAPIBLOCKCHAINEXPLORER_H

#include <fmt/format.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <api/Configuration.hpp>
#include <utils/hex.h>
#include <net/HttpClient.hpp>
#include <wallet/common/explorers/LedgerApiParser.hpp>
#include <wallet/common/Block.h>
#include <utils/JSONUtils.h>
#include <sstream>

namespace ledger {
    namespace core {

        //TODO: remove TransactionsParser and TransactionsBulkParser from template when refactoring them (common interface)
        template <typename BlockchainExplorerTransaction, typename TransactionsBulk, typename TransactionsParser, typename TransactionsBulkParser, typename BlockParser, typename NetworkParameters>
        class AbstractLedgerApiBlockchainExplorer {
        public:

            FuturePtr<TransactionsBulk>
            getLedgerApiTransactions(const std::vector<std::string> &addresses,
                            Option<std::string> fromBlockHash,
                            Option<void *> session) {
                auto joinedAddresses = Array<std::string>(addresses).join(strings::mkString(",")).getValueOr("");
                std::string params;
                std::unordered_map<std::string, std::string> headers;

                if (session.isEmpty()) {
                    params = "?noToken=true";
                } else {
                    headers["X-LedgerWallet-SyncToken"] = *((std::string *)session.getValue());
                }
                if (!fromBlockHash.isEmpty()) {
                    if (params.size() > 0) {
                        params = params + "&";
                    } else {
                        params = params + "?";
                    }
                    params = params + "blockHash=" + fromBlockHash.getValue();
                }
                return _http->GET(fmt::format("/blockchain/v2/{}/addresses/{}/transactions{}", getNetworkParameters().Identifier, joinedAddresses, params), headers)
                        .template json<TransactionsBulk, Exception>(LedgerApiParser<TransactionsBulk, TransactionsBulkParser>())
                        .template mapPtr<TransactionsBulk>(getExplorerContext(), [fromBlockHash] (const Either<Exception, std::shared_ptr<TransactionsBulk>>& result) {
                            if (result.isLeft()) {
                                if (fromBlockHash.isEmpty()) {
                                    throw result.getLeft();
                                } else {
                                    throw make_exception(api::ErrorCode::BLOCK_NOT_FOUND, "Unable to find block with hash {}", fromBlockHash.getValue());
                                }
                            } else {
                                return result.getRight();
                            }
                        });
            };

            FuturePtr<Block>
            getLedgerApiCurrentBlock() {
                return _http->GET(fmt::format("/blockchain/v2/{}/blocks/current", getNetworkParameters().Identifier))
                        .template json<Block, Exception>(LedgerApiParser<Block, BlockParser>())
                        .template mapPtr<Block>(getExplorerContext(), [] (const Either<Exception, std::shared_ptr<Block>>& result) {
                            if (result.isLeft()) {
                                throw result.getLeft();
                            } else {
                                return result.getRight();
                            }
                        });
            };

            FuturePtr<BlockchainExplorerTransaction>
            getLedgerApiTransactionByHash(const String &transactionHash) {
                return _http->GET(fmt::format("/blockchain/v2/{}/transactions/{}", getNetworkParameters().Identifier, transactionHash.str()))
                        .template json<std::vector<BlockchainExplorerTransaction>, Exception>(LedgerApiParser<std::vector<BlockchainExplorerTransaction>, TransactionsParser>())
                        .template mapPtr<BlockchainExplorerTransaction>(getExplorerContext(), [transactionHash] (const Either<Exception, std::shared_ptr<std::vector<BlockchainExplorerTransaction>>>& result) {
                            if (result.isLeft()) {
                                throw result.getLeft();
                            } else if (result.getRight()->size() == 0) {
                                throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction '{}' not found", transactionHash.str());
                            } else {
                                auto tx = (*result.getRight())[0];
                                auto transaction = std::make_shared<BlockchainExplorerTransaction>(tx);
                                return transaction;
                            }
                        });
            };

            Future<void *> startLedgerApiSession() {
                return _http->GET(fmt::format("/blockchain/v2/{}/syncToken", getNetworkParameters().Identifier))
                        .json().template map<void *>(getExplorerContext(), [] (const HttpRequest::JsonResult& result) {
                            auto& json = *std::get<1>(result);
                            return new std::string(json["token"].GetString(), json["token"].GetStringLength());
                        });
            };

            Future<Unit> killLedgerApiSession(void *session) {
                return _http->addHeader("X-LedgerWallet-SyncToken", *((std::string *)session))
                        .DEL(fmt::format("/blockchain/v2/{}/syncToken", getNetworkParameters().Identifier))
                        .json().template map<Unit>(getExplorerContext(), [] (const HttpRequest::JsonResult& result) {
                            return unit;
                        });
            };

            Future<String> pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
                std::stringstream body;
                body << "{" << "\"tx\":" << '"' << hex::toString(transaction) << '"' << "}";
                auto bodyString = body.str();
                return _http->POST(fmt::format("/blockchain/v2/{}/transactions/send", getNetworkParameters().Identifier),
                                   std::vector<uint8_t>(bodyString.begin(), bodyString.end())
                ).json().template map<String>(getExplorerContext(), [] (const HttpRequest::JsonResult& result) -> String {
                    auto& json = *std::get<1>(result);
                    return json["result"].GetString();
                });
            }

            Future<Bytes> getLedgerApiRawTransaction(const String& transactionHash) {
                return _http->GET(fmt::format("/blockchain/v2/{}/transactions/{}/hex", getNetworkParameters().Identifier, transactionHash.str()))
                        .json().template map<Bytes>(getExplorerContext(), [transactionHash] (const HttpRequest::JsonResult& result) {
                            auto& json = *std::get<1>(result);
                            if (json.GetArray().Size() == 0) {
                                throw make_exception(api::ErrorCode::RAW_TRANSACTION_NOT_FOUND, "Unable to retrieve {}", transactionHash.str());
                            } else {
                                auto& hex = json[0].GetObject()["hex"];
                                return Bytes(ledger::core::hex::toByteArray(std::string(hex.GetString(), hex.GetStringLength())));
                            }
                        });
            }

            Future<int64_t > getLedgerApiTimestamp() {
                auto delay = 60*getNetworkParameters().TimestampDelay;
                return _http->GET(fmt::format("/timestamp"))
                        .json().map<int64_t>(getExplorerContext(), [delay] (const HttpRequest::JsonResult& result) {
                            auto& json = *std::get<1>(result);
                            return json["timestamp"].GetInt64() - delay;
                        });
            }

        protected:
            virtual std::shared_ptr<api::ExecutionContext> getExplorerContext() = 0;
            virtual NetworkParameters getNetworkParameters() = 0;
            std::shared_ptr<HttpClient> _http;
        };
    }
}


#endif //LEDGER_CORE_ABSTRACTLEDGERAPIBLOCKCHAINEXPLORER_H
