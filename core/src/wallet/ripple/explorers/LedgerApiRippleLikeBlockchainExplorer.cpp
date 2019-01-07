/*
 *
 * LedgerApiRippleLikeBlockchainExplorer
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


#include "LedgerApiRippleLikeBlockchainExplorer.h"

namespace ledger {
    namespace core {

        LedgerApiRippleLikeBlockchainExplorer::LedgerApiRippleLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http,
                const api::RippleLikeNetworkParameters &parameters,
                const std::shared_ptr<api::DynamicObject> &configuration) :
                DedicatedContext(context),
                RippleLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http = http;
            _parameters = parameters;
            _explorerVersion = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION).value_or("v3");
        }

        Future<std::shared_ptr<BigInt>>
        LedgerApiRippleLikeBlockchainExplorer::getBalance(const std::vector<RippleLikeKeychain::Address> &addresses) {

            std::string addressesStr;
            auto size = addresses.size();
            for (auto i = 0; i < size; i++) {
                auto address = addresses[i];
                addressesStr += address->toBase58();
                if (i < addresses.size() - 1) {
                    addressesStr += ",";
                }
            }
            return _http->GET(fmt::format("/blockchain/{}/{}/addresses/{}/balance", _explorerVersion,
                                          _parameters.Identifier, addressesStr))
                    .json().mapPtr<BigInt>(getContext(), [addressesStr, size](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        if (!json.IsArray() || json.Size() != size || !json[0].HasMember("balance") ||
                            !json[0]["balance"].IsUint64()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get balance for {}",
                                                 addressesStr);
                        }

                        auto balance = json[0]["balance"].GetUint64();
                        for (auto i = 1; i < size; i++) {
                            if (json[i].HasMember("balance") || json[i]["balance"].IsUint64()) {
                                balance += json[i]["balance"].GetUint64();
                            }
                        }

                        return std::make_shared<BigInt>((int64_t) balance);
                    });
        }

        Future<String>
        LedgerApiRippleLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            std::stringstream body;
            body << "{" << "\"tx\":" << '"' << hex::toString(transaction) << '"' << "}";
            auto bodyString = body.str();
            return _http->POST(fmt::format("/blockchain/{}/{}/transactions/send", getExplorerVersion(),
                                           getNetworkParameters().Identifier),
                               std::vector<uint8_t>(bodyString.begin(), bodyString.end())
            ).json().template map<String>(getExplorerContext(), [](const HttpRequest::JsonResult &result) -> String {
                auto &json = *std::get<1>(result);
                return json["result"].GetString();
            });
        }

        Future<void *> LedgerApiRippleLikeBlockchainExplorer::startSession() {
            return startLedgerApiSession();
        }

        Future<Unit> LedgerApiRippleLikeBlockchainExplorer::killSession(void *session) {
            return killLedgerApiSession(session);
        }

        Future<Bytes> LedgerApiRippleLikeBlockchainExplorer::getRawTransaction(const String &transactionHash) {
            return getLedgerApiRawTransaction(transactionHash);
        }

        Future<String> LedgerApiRippleLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        FuturePtr<RippleLikeBlockchainExplorer::TransactionsBulk>
        LedgerApiRippleLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                               Option<std::string> fromBlockHash,
                                                               Option<void *> session) {
            return getLedgerApiTransactions(addresses, fromBlockHash, session);
        }

        FuturePtr<Block> LedgerApiRippleLikeBlockchainExplorer::getCurrentBlock() const {
            return getLedgerApiCurrentBlock();
        }

        FuturePtr<RippleLikeBlockchainExplorerTransaction>
        LedgerApiRippleLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t> LedgerApiRippleLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> LedgerApiRippleLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::RippleLikeNetworkParameters LedgerApiRippleLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string LedgerApiRippleLikeBlockchainExplorer::getExplorerVersion() const {
            return _explorerVersion;
        }
    }
}