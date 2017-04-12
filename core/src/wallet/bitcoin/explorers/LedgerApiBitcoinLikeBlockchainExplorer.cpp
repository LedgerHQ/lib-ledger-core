/*
 *
 * LedgerApiBitcoinLikeBlockchainExplorer
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/03/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "LedgerApiBitcoinLikeBlockchainExplorer.hpp"
#include <fmt/format.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "../../../utils/hex.h"
#include "src/wallet/bitcoin/explorers/api/TransactionParser.hpp"
#include "api/BlockParser.hpp"

namespace ledger {
    namespace core {

        LedgerApiBitcoinLikeBlockchainExplorer::LedgerApiBitcoinLikeBlockchainExplorer(
        const std::shared_ptr<api::ExecutionContext> &context,
        const std::shared_ptr<HttpClient> &http,
        const api::BitcoinLikeNetworkParameters& parameters) : DedicatedContext(context) {
            _http = http;
            _parameters = parameters;
        }

        Future<void *> LedgerApiBitcoinLikeBlockchainExplorer::startSession() {
            return _http
            ->GET(fmt::format("/blockchain/v2/{}/syncToken", _parameters.Identifier))
            .json().map<void *>(getContext(), [] (const HttpRequest::JsonResult& result) {
                auto& json = *std::get<1>(result);
                return new std::string(json["token"].GetString(), json["token"].GetStringLength());
            });
        }

        Future<Unit> LedgerApiBitcoinLikeBlockchainExplorer::killSession(void *session) {
            Promise<Unit> promise;
            return promise.getFuture();
        }

        Future<String> LedgerApiBitcoinLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            Promise<String> promise;
            return promise.getFuture();
        }

        Future<Bytes> LedgerApiBitcoinLikeBlockchainExplorer::getRawTransaction(const String& transactionHash) {
            return _http
            ->GET(fmt::format("/blockchain/v2/{}/transactions/{}/hex", _parameters.Identifier, transactionHash.str()))
            .json().map<Bytes>(getContext(), [transactionHash] (const HttpRequest::JsonResult& result) {
                auto& json = *std::get<1>(result);
                if (json.GetArray().Size() == 0) {
                    throw make_exception(api::ErrorCode::RAW_TRANSACTION_NOT_FOUND, "Unable to retrieve {}", transactionHash.str());
                } else {
                    auto& hex = json[0].GetObject()["hex"];
                    return Bytes(ledger::core::hex::toByteArray(std::string(hex.GetString(), hex.GetStringLength())));
                }
            });
        }

        Future<BitcoinLikeBlockchainExplorer::TransactionsBulk>
        LedgerApiBitcoinLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                                Option<std::string> fromBlockHash,
                                                                Option<void *> session) {
            Promise<BitcoinLikeBlockchainExplorer::TransactionsBulk> promise;
            return promise.getFuture();
        }

        Future<BitcoinLikeBlockchainExplorer::Block> LedgerApiBitcoinLikeBlockchainExplorer::getCurrentBlock() {
            Promise<BitcoinLikeBlockchainExplorer::Block> promise;
            return promise.getFuture();
        }

        Future<BitcoinLikeBlockchainExplorer::Transaction>
        LedgerApiBitcoinLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) {
            Promise<BitcoinLikeBlockchainExplorer::Transaction> promise;
            return _http
                ->GET(fmt::format("/blockchain/v2/{}/transactions/{}", _parameters.Identifier, transactionHash.str()))
                .json<BitcoinLikeBlockchainExplorer::Transaction, Exception>(TransactionParser())
            .map<BitcoinLikeBlockchainExplorer::Transaction>(_executionContext, [] (const Either<Exception, BitcoinLikeBlockchainExplorer::Transaction>& result) {
                if (result.isLeft()) {
                    throw result.getLeft();
                } else {
                    BitcoinLikeBlockchainExplorer::Transaction transaction = result.getRight();
                    return transaction;
                }
            });
        }

    }
}