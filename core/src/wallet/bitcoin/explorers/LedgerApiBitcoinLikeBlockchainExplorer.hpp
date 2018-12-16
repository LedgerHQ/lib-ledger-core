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
#ifndef LEDGER_CORE_LEDGERAPIBITCOINLIKEBLOCKCHAINEXPLORER_HPP
#define LEDGER_CORE_LEDGERAPIBITCOINLIKEBLOCKCHAINEXPLORER_HPP


#include <wallet/common/explorers/AbstractLedgerApiBlockchainExplorer.h>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <api/BitcoinLikeNetworkParameters.hpp>
#include <collections/collections.hpp>
#include <async/Future.hpp>
#include <net/HttpClient.hpp>
#include <async/DedicatedContext.hpp>
#include "api/TransactionParser.hpp"
#include "api/TransactionsBulkParser.hpp"
#include "api/BlockParser.hpp"
#include <api/BitcoinLikeNetworkParameters.hpp>

namespace ledger {
    namespace core {

        using LedgerApiBlockchainExplorer = AbstractLedgerApiBlockchainExplorer<BitcoinLikeBlockchainExplorerTransaction, BitcoinLikeBlockchainExplorer::TransactionsBulk, TransactionsParser, TransactionsBulkParser, BlockParser, api::BitcoinLikeNetworkParameters>;
        class LedgerApiBitcoinLikeBlockchainExplorer : public BitcoinLikeBlockchainExplorer,
                                                       public LedgerApiBlockchainExplorer,
                                                       public DedicatedContext,
                                                       public std::enable_shared_from_this<LedgerApiBitcoinLikeBlockchainExplorer> {
        public:
            LedgerApiBitcoinLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext>& context,
                const std::shared_ptr<HttpClient>& http,
                const api::BitcoinLikeNetworkParameters& parameters,
                const std::shared_ptr<api::DynamicObject>& configuration
            );

            Future<String> pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) override;
            Future<void *> startSession() override;
            Future<Unit> killSession(void *session) override;
            Future<Bytes> getRawTransaction(const String& transactionHash) override;
            Future<String> pushTransaction(const std::vector<uint8_t>& transaction) override;

            FuturePtr<TransactionsBulk>
            getTransactions(const std::vector<std::string> &addresses,
                            Option<std::string> fromBlockHash = Option<std::string>(),
                            Option<void *> session = Option<void *>()) override;

            FuturePtr<Block> getCurrentBlock() override;

            FuturePtr<BitcoinLikeBlockchainExplorerTransaction> getTransactionByHash(const String &transactionHash) override;

            Future<int64_t > getTimestamp() override;

            std::shared_ptr<api::ExecutionContext> getExplorerContext() override;
            api::BitcoinLikeNetworkParameters getNetworkParameters() override;
            std::string getExplorerVersion() override;
        private:
            api::BitcoinLikeNetworkParameters _parameters;
            std::string _explorerVersion;
        };
    }
}


#endif //LEDGER_CORE_LEDGERAPIBITCOINLIKEBLOCKCHAINEXPLORER_HPP
