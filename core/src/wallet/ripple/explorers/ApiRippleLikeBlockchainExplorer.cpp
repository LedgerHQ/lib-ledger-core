/*
 *
 * ApiRippleLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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


#include "ApiRippleLikeBlockchainExplorer.h"

namespace ledger {
    namespace core {

        ApiRippleLikeBlockchainExplorer::ApiRippleLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http,
                const api::RippleLikeNetworkParameters &parameters,
                const std::shared_ptr<api::DynamicObject> &configuration) :
                DedicatedContext(context),
                RippleLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http = http;
            _parameters = parameters;
            _explorerVersion = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION).value_or("v2");
        }

        Future<String>
        ApiRippleLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            throw make_exception(api::ErrorCode::MISSING_DERIVATION, "Missing implementation of pushTransaction method for Ripple API");
        }

        Future<std::shared_ptr<BigInt>>
        ApiRippleLikeBlockchainExplorer::getBalance(const std::vector<RippleLikeKeychain::Address> &addresses) {

            //TODO: multiple accounts balances ?
            if (addresses.size() != 1) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "ApiRippleLikeBlockchainExplorer::getBalance can called only with one address");
            }

            std::string addressesStr = addresses[0]->toBase58();
            auto size = addresses.size();

            return _http->GET(fmt::format("/{}/accounts/{}/balances", _explorerVersion, addressesStr))
                    .json().mapPtr<BigInt>(getContext(), [addressesStr, size](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        if (!json.IsObject() || !json.HasMember("balances") ||
                            !json["balances"].IsArray()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get balance for {}",
                                                 addressesStr);
                        }
                        std::string strBalance;
                        auto size = json["balances"].Size();
                        //Find XRP balance
                        for (int i = 0; i < size; i++) {
                            if (json["balances"][i]["currency"] == "XRP") {
                                strBalance = json["balances"][i]["value"].GetString();
                                break;
                            }
                        }
                        return std::make_shared<BigInt>(strBalance);
                    });
        }


        Future<void *> ApiRippleLikeBlockchainExplorer::startSession() {
            return Future<void *>::successful(new std::string("", 0));
        }

        Future<Unit> ApiRippleLikeBlockchainExplorer::killSession(void *session) {
            return Future<Unit>::successful(unit);
        }

        Future<Bytes> ApiRippleLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            return getLedgerApiRawTransaction(transactionHash);
        }

        Future<String> ApiRippleLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        FuturePtr<RippleLikeBlockchainExplorer::TransactionsBulk>
        ApiRippleLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                               Option<std::string> fromBlockHash,
                                                               Option<void *> session) {
            auto joinedAddresses = Array<std::string>(addresses).join(strings::mkString(",")).getValueOr("");
            std::string params;
            std::unordered_map<std::string, std::string> headers;

            if (!fromBlockHash.isEmpty()) {
                if (params.size() > 0) {
                    params = params + "&";
                } else {
                    params = params + "?";
                }
                params = params + "blockHash=" + fromBlockHash.getValue();
            }
            return _http->GET(fmt::format("/{}/accounts/{}/transactions{}", getExplorerVersion(), joinedAddresses, params), headers)
                    .template json<TransactionsBulk, Exception>(LedgerApiParser<TransactionsBulk, RippleLikeTransactionsBulkParser>())
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
        }

        FuturePtr<Block> ApiRippleLikeBlockchainExplorer::getCurrentBlock() const {
            return _http->GET(fmt::format("/{}/ledgers", getExplorerVersion()))
                    .template json<Block, Exception>(LedgerApiParser<Block, RippleLikeBlockParser>())
                    .template mapPtr<Block>(getExplorerContext(), [] (const Either<Exception, std::shared_ptr<Block>>& result) {
                        if (result.isLeft()) {
                            throw result.getLeft();
                        } else {
                            return result.getRight();
                        }
                    });
        }

        FuturePtr<RippleLikeBlockchainExplorerTransaction>
        ApiRippleLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t> ApiRippleLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> ApiRippleLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::RippleLikeNetworkParameters ApiRippleLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string ApiRippleLikeBlockchainExplorer::getExplorerVersion() const {
            return _explorerVersion;
        }
    }
}