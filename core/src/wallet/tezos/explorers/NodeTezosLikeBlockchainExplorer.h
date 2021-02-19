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

#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/tezos/explorers/api/TezosLikeTransactionsParser.h>
#include <wallet/tezos/explorers/api/TezosLikeTransactionsBulkParser.h>
#include <wallet/tezos/explorers/api/TezosLikeBlockParser.h>
#include <api/TezosLikeNetworkParameters.hpp>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>

namespace ledger {
    namespace core {

        class NodeTezosLikeBlockchainExplorer : public TezosLikeBlockchainExplorer
        {
        public:
            NodeTezosLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http,
                const api::TezosLikeNetworkParameters &parameters,
                const std::shared_ptr<ledger::core::api::DynamicObject> &configuration);

            Future<std::shared_ptr<BigInt>>
            getBalance(const TezosLikeKeychain::Address &address) const override;

            Future<std::shared_ptr<BigInt>>
            getFees() const override;

            Future<std::shared_ptr<BigInt>>
            getGasPrice() const override;

            Future<String>
            pushLedgerApiTransaction(const std::vector<uint8_t> &transaction) override;

            FuturePtr<TransactionsBulk>
            getTransactions(const std::string& address,
                            const Either<std::string, uint32_t>& token = {}) const override;

            FuturePtr<Block>
            getCurrentBlock() const override;

            std::string getExplorerVersion() const override;

            Future<std::shared_ptr<BigInt>>
            getEstimatedGasLimit(const std::string &address) const override;

            Future<std::shared_ptr<GasLimit>>
            getEstimatedGasLimit(const std::shared_ptr<TezosLikeTransactionApi> &tx) const override;

            Future<std::shared_ptr<BigInt>>
            getStorage(const std::string &address) const override;

            Future<std::vector<uint8_t>>
            forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx) const override;

            Future<std::string>
            getManagerKey(const std::string &address) const override;

            Future<bool>
            isAllocated(const std::string &address) const override;

            Future<std::string>
            getCurrentDelegate(const std::string &address) const override;


            Future<std::shared_ptr<BigInt>>
            getCounter(const std::string &address) const override;

            Future<bool>
            isFunded(const std::string &address) const override;

            Future<bool>
            isDelegate(const std::string &address) const override;

            Future<std::shared_ptr<BigInt>>
            getTokenBalance(const std::string& accountAddress,
                            const std::string& tokenAddress) const override;

        private:
            /*
             * Helper to a get specific field's value from given url
             * WARNING: this is only useful for fields with an integer (decimal representation) value (with a string type)
             * @param url : base url to fetch the value on,
             * @param field: name of field we are interested into,
             * @param params: additional params to query value of field
             * @return BigInt representing the value of targetted field
             */
            Future<std::shared_ptr<BigInt>>
            getHelper(const std::string &url,
                      const std::string &field,
                      const std::unordered_map<std::string, std::string> &params = {},
                      const std::string &fallbackValue = "") const;

        private:
            std::string _explorerVersion;
        };

    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_NODETEZOSLIKEBLOCKCHAINEXPLORER_H
