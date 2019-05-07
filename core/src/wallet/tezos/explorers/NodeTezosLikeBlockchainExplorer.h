/*
 *
 * NodeTezosLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 29/04/2019.
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


#ifndef LEDGER_CORE_NODETEZOSLIKEBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_NODETEZOSLIKEBLOCKCHAINEXPLORER_H

#include <wallet/common/explorers/AbstractLedgerApiBlockchainExplorer.h>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/tezos/explorers/api/TezosLikeTransactionsParser.h>
#include <wallet/tezos/explorers/api/TezosLikeTransactionsBulkParser.h>
#include <wallet/tezos/explorers/api/TezosLikeBlockParser.h>
#include <api/TezosLikeNetworkParameters.hpp>

namespace ledger {
    namespace core {
        using LedgerApiBlockchainExplorer = AbstractLedgerApiBlockchainExplorer<TezosLikeBlockchainExplorerTransaction, TezosLikeBlockchainExplorer::TransactionsBulk, TezosLikeTransactionsParser, TezosLikeTransactionsBulkParser, TezosLikeBlockParser, api::TezosLikeNetworkParameters>;

        class NodeTezosLikeBlockchainExplorer : public TezosLikeBlockchainExplorer,
                                                 public LedgerApiBlockchainExplorer,
                                                 public DedicatedContext,
                                                 public std::enable_shared_from_this<NodeTezosLikeBlockchainExplorer> {
        public:
            NodeTezosLikeBlockchainExplorer(const std::shared_ptr<api::ExecutionContext> &context,
                                             const std::shared_ptr<HttpClient> &http,
                                             const api::TezosLikeNetworkParameters &parameters,
                                             const std::shared_ptr<api::DynamicObject> &configuration);

            Future<std::shared_ptr<BigInt>>
            getBalance(const std::vector<TezosLikeKeychain::Address> &addresses) override;

            Future<std::shared_ptr<BigInt>>
            getFees() override;

            Future<String> pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) override;

            Future<void *> startSession() override;

            Future<Unit> killSession(void *session) override;

            Future<Bytes> getRawTransaction(const String &transactionHash) override;

            Future<String> pushTransaction(const std::vector<uint8_t> &transaction) override;

            FuturePtr<TezosLikeBlockchainExplorer::TransactionsBulk>
            getTransactions(const std::vector<std::string> &addresses,
                            Option<std::string> fromBlockHash = Option<std::string>(),
                            Option<void *> session = Option<void *>()) override;

            FuturePtr<Block> getCurrentBlock() const override;

            FuturePtr<TezosLikeBlockchainExplorerTransaction>
            getTransactionByHash(const String &transactionHash) const override;

            Future<int64_t> getTimestamp() const override;

            std::shared_ptr<api::ExecutionContext> getExplorerContext() const override;

            api::TezosLikeNetworkParameters getNetworkParameters() const override;

            std::string getExplorerVersion() const override;

            Future<std::shared_ptr<BigInt>>
            getEstimatedGasLimit(const std::string &address) override;

            Future<std::shared_ptr<BigInt>>
            getStorage(const std::string &address) override;

            Future<std::shared_ptr<BigInt>> getCounter(const std::string &address) override;

        private:
            api::TezosLikeNetworkParameters _parameters;
        };
    }
}
#endif //LEDGER_CORE_NODETEZOSLIKEBLOCKCHAINEXPLORER_H
