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

#include <core/api/ErrorCode.hpp>
#include <core/math/BigInt.hpp>

#include <bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {

        LedgerApiBitcoinLikeBlockchainExplorer::LedgerApiBitcoinLikeBlockchainExplorer(const std::shared_ptr<api::ExecutionContext> &context,
                                                                                       const std::shared_ptr<HttpClient> &http,
                                                                                       const api::BitcoinLikeNetworkParameters& parameters,
                                                                                       const std::shared_ptr<api::DynamicObject>& configuration) :
                DedicatedContext(context),
                BitcoinLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http = http;
            _parameters = parameters;
            _explorerVersion = configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_VERSION).value_or("v2");
        }

        Future<String> LedgerApiBitcoinLikeBlockchainExplorer::pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) {
            std::stringstream body;
            body << "{" << "\"tx\":" << '"' << hex::toString(transaction) << '"' << "}";
            auto bodyString = body.str();
            return _http->POST(fmt::format("/blockchain/{}/{}/transactions/send", getExplorerVersion(), getNetworkParameters().Identifier),
                               std::vector<uint8_t>(bodyString.begin(), bodyString.end())
            ).json().template map<String>(getExplorerContext(), [] (const HttpRequest::JsonResult& result) -> String {
                auto& json = *std::get<1>(result);
                return json["result"].GetString();
            });
        }

        Future<void *> LedgerApiBitcoinLikeBlockchainExplorer::startSession() {
            return startLedgerApiSession();
        }

        Future<Unit> LedgerApiBitcoinLikeBlockchainExplorer::killSession(void *session) {
            return killLedgerApiSession(session);
        }

        Future<String> LedgerApiBitcoinLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t> &transaction) {
            return pushLedgerApiTransaction(transaction);
        }

        Future<Bytes> LedgerApiBitcoinLikeBlockchainExplorer::getRawTransaction(const String& transactionHash) {
            return getLedgerApiRawTransaction(transactionHash);
        }

        FuturePtr<BitcoinLikeBlockchainExplorer::TransactionsBulk>
        LedgerApiBitcoinLikeBlockchainExplorer::getTransactions(const std::vector<std::string> &addresses,
                                                                Option<std::string> fromBlockHash,
                                                                Option<void *> session) {
            bool const isSnakeCase = _explorerVersion == "v3";
            return getLedgerApiTransactions(addresses, fromBlockHash, session, isSnakeCase);
        }

        FuturePtr<BitcoinLikeBlockchainExplorer::Block> LedgerApiBitcoinLikeBlockchainExplorer::getCurrentBlock() const {
            return getLedgerApiCurrentBlock();
        }

        FuturePtr<BitcoinLikeBlockchainExplorerTransaction>
        LedgerApiBitcoinLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) const {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t > LedgerApiBitcoinLikeBlockchainExplorer::getTimestamp() const {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> LedgerApiBitcoinLikeBlockchainExplorer::getExplorerContext() const {
            return _executionContext;
        }

        api::BitcoinLikeNetworkParameters LedgerApiBitcoinLikeBlockchainExplorer::getNetworkParameters() const {
            return _parameters;
        }

        std::string LedgerApiBitcoinLikeBlockchainExplorer::getExplorerVersion() const {
            return _explorerVersion;
        }

        Future<std::vector<std::shared_ptr<api::BigInt>>> LedgerApiBitcoinLikeBlockchainExplorer::getFees() {
            bool parseNumbersAsString = true;
            auto networkId = getNetworkParameters().Identifier;
            return _http->GET(fmt::format("/blockchain/{}/{}/fees", getExplorerVersion(), networkId))
                    .json(parseNumbersAsString).map<std::vector<std::shared_ptr<api::BigInt>>>(getExplorerContext(), [networkId] (const HttpRequest::JsonResult& result) {
                        auto& json = *std::get<1>(result);
                        if (!json.IsObject()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get fees for {}", networkId);
                        }

                        // Here we filter fields returned by this endpoint,
                        // if the field's key is a number (number of confirmations) then it's an acceptable fee
                        auto isValid = [] (const std::string &field) -> bool {
                            return !field.empty() && std::find_if(field.begin(), field.end(), [](char c) { return !std::isdigit(c); }) == field.end();
                        };
                        std::vector<std::shared_ptr<api::BigInt>> fees;
                        for (auto& item : json.GetObject()) {
                            if (item.name.IsString() && item.value.IsString() && isValid(item.name.GetString())){
                                fees.push_back(std::make_shared<BigInt>(BigInt::fromString(item.value.GetString())));
                            }
                        }
                        return fees;
                    });
        }

    }
}
