/*
 *
 * NodeRippleLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 09/01/2019.
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

#pragma once

#include <core/explorers/AbstractLedgerApiBlockchainExplorer.hpp>

#include <ripple/api/RippleLikeNetworkParameters.hpp>
#include <ripple/explorers/RippleLikeBlockchainExplorer.hpp>
#include <ripple/explorers/RippleLikeTransactionsParser.hpp>
#include <ripple/explorers/RippleLikeTransactionsBulkParser.hpp>
#include <ripple/explorers/RippleLikeBlockParser.hpp>

namespace ledger {
    namespace core {
        using LedgerApiBlockchainExplorer = AbstractLedgerApiBlockchainExplorer<RippleLikeBlockchainExplorerTransaction, RippleLikeBlockchainExplorer::TransactionsBulk, RippleLikeTransactionsParser, RippleLikeTransactionsBulkParser, RippleLikeBlockParser, api::RippleLikeNetworkParameters>;

        // Fields we are interested into are numbers or strings
        enum FieldTypes {
            NumberType,
            StringType
        };

        class NodeRippleLikeBlockchainExplorer : public RippleLikeBlockchainExplorer,
                                                 public LedgerApiBlockchainExplorer,
                                                 public DedicatedContext,
                                                 public std::enable_shared_from_this<NodeRippleLikeBlockchainExplorer> {
        public:
            NodeRippleLikeBlockchainExplorer(const std::shared_ptr<api::ExecutionContext> &context,
                                             const std::shared_ptr<HttpClient> &http,
                                             const api::RippleLikeNetworkParameters &parameters,
                                             const std::shared_ptr<api::DynamicObject> &configuration);

            Future<std::shared_ptr<BigInt>>
            getBalance(const std::vector<RippleLikeKeychain::Address> &addresses) override;

            Future<std::shared_ptr<BigInt>>
            getSequence(const std::string &address) override;

            Future<std::shared_ptr<BigInt>>
            getFees() override;

            Future<std::shared_ptr<BigInt>>
            getBaseReserve() override;

            Future<std::shared_ptr<BigInt>>
            getLedgerSequence() override;

            Future<std::shared_ptr<BigInt>>
            getServerInfo(const std::string &field, FieldTypes type);

            Future<String> pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) override;

            Future<void *> startSession() override;

            Future<Unit> killSession(void *session) override;

            Future<Bytes> getRawTransaction(const String &transactionHash) override;

            Future<String> pushTransaction(const std::vector<uint8_t> &transaction) override;

            FuturePtr<RippleLikeBlockchainExplorer::TransactionsBulk>
            getTransactions(const std::vector<std::string> &addresses,
                            Option<std::string> fromBlockHash = Option<std::string>(),
                            Option<void *> session = Option<void *>()) override;

            FuturePtr<Block> getCurrentBlock() const override;

            FuturePtr<RippleLikeBlockchainExplorerTransaction>
            getTransactionByHash(const String &transactionHash) const override;

            Future<int64_t> getTimestamp() const override;

            std::shared_ptr<api::ExecutionContext> getExplorerContext() const override;

            api::RippleLikeNetworkParameters getNetworkParameters() const override;

            std::string getExplorerVersion() const override;

        private:
            Future<std::shared_ptr<BigInt>>
            getAccountInfo(const std::string &address,
                           const std::string &key,
                           const BigInt &defaultValue,
                           FieldTypes);

            api::RippleLikeNetworkParameters _parameters;
        };
    }
}
