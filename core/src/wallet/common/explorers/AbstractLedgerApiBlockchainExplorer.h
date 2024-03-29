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

#include <api/Configuration.hpp>
#include <fmt/format.h>
#include <net/HttpClient.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <utils/JSONUtils.h>
#include <utils/hex.h>
#include <wallet/common/Block.h>
#include <wallet/common/explorers/LedgerApiParser.hpp>

namespace ledger {
    namespace core {

        // TODO: remove TransactionsParser and TransactionsBulkParser from template when refactoring them (common interface)
        template <typename BlockchainExplorerTransaction, typename TransactionsBulk, typename TransactionsParser, typename TransactionsBulkParser, typename BlockParser, typename NetworkParameters>
        class AbstractLedgerApiBlockchainExplorer {
            constexpr static const uint16_t DEFAULT_BATCH_SIZE = 1000;

          private:
            std::string getParams(Option<std::string> fromBlockHash,
                                  const Option<void *> &session,
                                  bool isSnakeCase    = false,
                                  uint16_t batch_size = DEFAULT_BATCH_SIZE) {
                std::string params;
                std::unordered_map<std::string, std::string> headers;

                if (session.isEmpty()) {
                    params = isSnakeCase ? "?no_token=true" : "?noToken=true";
                }

                // Block hash
                if (!fromBlockHash.isEmpty()) {
                    if (!params.empty()) {
                        params = params + "&";
                    } else {
                        params = params + "?";
                    }
                    const std::string blockHash = isSnakeCase ? "block_hash=" : "blockHash=";
                    params                      = params + blockHash + fromBlockHash.getValue();
                }

                // Batch size
                if (!params.empty()) {
                    params = params + "&";
                } else {
                    params = params + "?";
                }
                params = params + "batch_size=" + std::to_string(batch_size);
                return params;
            }

          public:
            FuturePtr<TransactionsBulk>
            getLedgerApiTransactions(const std::vector<std::string> &addresses,
                                     Option<std::string> fromBlockHash,
                                     Option<void *> session,
                                     bool isSnakeCase    = false,
                                     uint16_t batch_size = DEFAULT_BATCH_SIZE) {
                auto joinedAddresses = Array<std::string>(addresses).join(strings::mkString(",")).getValueOr("");
                std::string params   = getParams(fromBlockHash, session, isSnakeCase, batch_size);
                std::unordered_map<std::string, std::string> headers;

                if (!session.isEmpty()) {
                    headers["X-LedgerWallet-SyncToken"] = *((std::string *)session.getValue());
                }

                return _http->GET(fmt::format("/blockchain/{}/{}/addresses/{}/transactions{}", getExplorerVersion(), getNetworkParameters().Identifier, joinedAddresses, params), headers)
                    .template json<TransactionsBulk, Exception>(LedgerApiParser<TransactionsBulk, TransactionsBulkParser>(), true)
                    .template mapPtr<TransactionsBulk>(getExplorerContext(), [fromBlockHash, batch_size](const Either<Exception, std::shared_ptr<TransactionsBulk>> &result) {
                        if (result.isLeft()) {
                            // Only case where we should emit block not found error
                            if (!fromBlockHash.isEmpty() && result.getLeft().getErrorCode() == api::ErrorCode::HTTP_ERROR) {
                                throw make_exception(api::ErrorCode::BLOCK_NOT_FOUND, "Unable to find block with hash {}", fromBlockHash.getValue());
                            } else {
                                throw result.getLeft();
                            }
                        }

                        auto bulk = result.getRight();
                        if (bulk->transactions.empty() || !bulk->transactions.front().block.hasValue() || !bulk->transactions.back().block.hasValue()) {
                            return bulk;
                        }
                        auto firstBlock = bulk->transactions.front().block.getValue();
                        auto lastBlock  = bulk->transactions.back().block.getValue();
                        if (bulk->transactions.size() == batch_size && firstBlock.hash == lastBlock.hash) { // We might have more than {batch_size} transactions in a single block for this address, let's ask for more transaction ! (recovered exception below)
                            throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Some transaction might be missing !");
                        }
                        return result.getRight();
                    })
                    .recoverWith(getExplorerContext(), [this, addresses, fromBlockHash, session, isSnakeCase, batch_size](const Exception &e) {
                        if (e.getErrorCode() != api::ErrorCode::INCOMPLETE_TRANSACTION) {
                            throw e;
                        }
                        return getLedgerApiTransactions(addresses, fromBlockHash, session, isSnakeCase, batch_size + DEFAULT_BATCH_SIZE);
                    });
            };

            FuturePtr<Block>
            getLedgerApiCurrentBlock() const {
                return _http->GET(fmt::format("/blockchain/{}/{}/blocks/current", getExplorerVersion(), getNetworkParameters().Identifier))
                    .template json<Block, Exception>(LedgerApiParser<Block, BlockParser>())
                    .template mapPtr<Block>(getExplorerContext(), [](const Either<Exception, std::shared_ptr<Block>> &result) {
                        if (result.isLeft()) {
                            throw result.getLeft();
                        } else {
                            return result.getRight();
                        }
                    });
            };

            FuturePtr<BlockchainExplorerTransaction>
            getLedgerApiTransactionByHash(const String &transactionHash) const {
                return _http->GET(fmt::format("/blockchain/{}/{}/transactions/{}", getExplorerVersion(), getNetworkParameters().Identifier, transactionHash.str()))
                    .template json<std::vector<BlockchainExplorerTransaction>, Exception>(LedgerApiParser<std::vector<BlockchainExplorerTransaction>, TransactionsParser>())
                    .template mapPtr<BlockchainExplorerTransaction>(getExplorerContext(), [transactionHash](const Either<Exception, std::shared_ptr<std::vector<BlockchainExplorerTransaction>>> &result) {
                        if (result.isLeft()) {
                            throw result.getLeft();
                        } else if (result.getRight()->size() == 0) {
                            throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction '{}' not found", transactionHash.str());
                        } else {
                            auto tx          = (*result.getRight())[0];
                            auto transaction = std::make_shared<BlockchainExplorerTransaction>(tx);
                            return transaction;
                        }
                    });
            };

            Future<void *> startLedgerApiSession() const {
                return _http->GET(fmt::format("/blockchain/{}/{}/syncToken", getExplorerVersion(), getNetworkParameters().Identifier))
                    .json()
                    .template map<void *>(getExplorerContext(), [](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        return new std::string(json["token"].GetString(), json["token"].GetStringLength());
                    });
            };

            Future<Unit> killLedgerApiSession(void *session) const {
                return _http->addHeader("X-LedgerWallet-SyncToken", *((std::string *)session))
                    .DEL(fmt::format("/blockchain/{}/{}/syncToken", getExplorerVersion(), getNetworkParameters().Identifier))
                    .json()
                    .template map<Unit>(getExplorerContext(), [session](const HttpRequest::JsonResult &result) {
                        delete reinterpret_cast<std::string *>(session);
                        return unit;
                    });
            };

            Future<Bytes> getLedgerApiRawTransaction(const String &transactionHash) const {
                return _http->GET(fmt::format("/blockchain/{}/{}/transactions/{}/hex", getExplorerVersion(), getNetworkParameters().Identifier, transactionHash.str()))
                    .json()
                    .template map<Bytes>(getExplorerContext(), [transactionHash](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        if (json.GetArray().Size() == 0) {
                            throw make_exception(api::ErrorCode::RAW_TRANSACTION_NOT_FOUND, "Unable to retrieve {}", transactionHash.str());
                        } else {
                            auto &hex = json[0].GetObject()["hex"];
                            return Bytes(ledger::core::hex::toByteArray(std::string(hex.GetString(), hex.GetStringLength())));
                        }
                    });
            }

            Future<int64_t> getLedgerApiTimestamp() const {
                auto delay = 60 * getNetworkParameters().TimestampDelay;
                return _http->GET(fmt::format("/timestamp"))
                    .json()
                    .template map<int64_t>(getExplorerContext(), [delay](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        return json["timestamp"].GetInt64() - delay;
                    });
            }

            virtual Future<String> pushLedgerApiTransaction(const std::vector<uint8_t> &transaction, const std::string &correlationId = "") = 0;

          protected:
            virtual std::shared_ptr<api::ExecutionContext> getExplorerContext() const = 0;
            virtual NetworkParameters getNetworkParameters() const                    = 0;
            virtual std::string getExplorerVersion() const                            = 0;
            std::shared_ptr<HttpClient> _http;
        };
    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ABSTRACTLEDGERAPIBLOCKCHAINEXPLORER_H
