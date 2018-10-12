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
            return getLedgerApiTransactions(addresses, fromBlockHash, session);
        }

        FuturePtr<BitcoinLikeBlockchainExplorer::Block> LedgerApiBitcoinLikeBlockchainExplorer::getCurrentBlock() {
            return getLedgerApiCurrentBlock();
        }

        FuturePtr<BitcoinLikeBlockchainExplorerTransaction>
        LedgerApiBitcoinLikeBlockchainExplorer::getTransactionByHash(const String &transactionHash) {
            return getLedgerApiTransactionByHash(transactionHash);
        }

        Future<int64_t > LedgerApiBitcoinLikeBlockchainExplorer::getTimestamp() {
            return getLedgerApiTimestamp();
        }

        std::shared_ptr<api::ExecutionContext> LedgerApiBitcoinLikeBlockchainExplorer::getExplorerContext() {
            return _executionContext;
        }

        api::BitcoinLikeNetworkParameters LedgerApiBitcoinLikeBlockchainExplorer::getNetworkParameters() {
            return _parameters;
        }

        std::string LedgerApiBitcoinLikeBlockchainExplorer::getExplorerVersion() {
            return _explorerVersion;
        }

    }
}