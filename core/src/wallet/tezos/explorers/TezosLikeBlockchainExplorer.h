/*
 *
 * TezosLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#ifndef LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORER_H

#include <api/DynamicObject.hpp>
#include <api/ExecutionContext.hpp>
#include <api/TezosLikeNetworkParameters.hpp>
#include <api/TezosOperationTag.hpp>
#include <async/DedicatedContext.hpp>
#include <async/Future.hpp>
#include <math/BigInt.h>
#include <net/HttpClient.hpp>
#include <utils/ConfigurationMatchable.h>
#include <utils/Option.hpp>

#include <wallet/common/Block.h>
#include <wallet/common/explorers/AbstractLedgerApiBlockchainExplorer.h>
#include <wallet/tezos/keychains/TezosLikeKeychain.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace ledger {
    namespace core {

        struct TezosLikeBlockchainExplorerOriginatedAccount {
            TezosLikeBlockchainExplorerOriginatedAccount(const std::string &a = "",
                                                         bool isSpendable = false,
                                                         bool isDelegatable = false) :
                    address(a),
                    spendable(isSpendable),
                    delegatable(isDelegatable)
            {}

            std::string address;
            bool spendable;
            bool delegatable;
        };

        struct TezosLikeBlockchainExplorerTransaction {
            std::string hash;
            std::chrono::system_clock::time_point receivedAt;
            BigInt value;
            BigInt fees;
            BigInt gas_limit;
            BigInt storage_limit;
            std::string receiver;
            std::string sender;
            Option<Block> block;
            uint64_t confirmations;
            api::TezosOperationTag type;
            Option<std::string> publicKey;
            Option<TezosLikeBlockchainExplorerOriginatedAccount> originatedAccount;
            uint64_t status;
            std::string originatedAccountUid;
            std::string originatedAccountAddress;

            TezosLikeBlockchainExplorerTransaction() {
                confirmations = 0;
                type = api::TezosOperationTag::OPERATION_TAG_NONE;
                status = 0;
            }
        };

        struct GasLimit {
            BigInt reveal;
            BigInt transaction;

            GasLimit() :
                reveal(0), transaction(0) {}

            GasLimit(const BigInt& r, const BigInt& t) :
                reveal(r), transaction(t) {}
        };

        struct TezosLikeBlchainExplorerTransactionsBulk {
            std::vector<TezosLikeBlockchainExplorerTransaction> transactions;
            bool hasNext{false};
        };


        class TezosLikeTransactionsParser;
        class TezosLikeTransactionsBulkParser;
        class TezosLikeBlockParser;

        using TezosLikeLedgerApiBlockchainExplorer = AbstractLedgerApiBlockchainExplorer<
                TezosLikeBlockchainExplorerTransaction,
                TezosLikeBlchainExplorerTransactionsBulk,
                TezosLikeTransactionsParser,
                TezosLikeTransactionsBulkParser,
                TezosLikeBlockParser,
                api::TezosLikeNetworkParameters>;

        class TezosLikeTransactionApi;
        class TezosLikeBlockchainExplorer : protected TezosLikeLedgerApiBlockchainExplorer,
                                            public ConfigurationMatchable,
                                            public DedicatedContext
        {
        public:
            using Block = ledger::core::Block;
            using Transaction = TezosLikeBlockchainExplorerTransaction;
            using TransactionsBulk = TezosLikeBlchainExplorerTransactionsBulk;

            TezosLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http,
                const api::TezosLikeNetworkParameters &parameters,
                const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
                const std::vector<std::string> &matchableKeys);

            Future<String>
            pushTransaction(const std::vector<uint8_t> &transaction);

            FuturePtr<Transaction>
            getTransactionByHash(const String &transactionHash) const;

            virtual Future<std::shared_ptr<BigInt>>
            getBalance(const TezosLikeKeychain::Address &address) const = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getFees() const = 0;

            /// Return the gas Price of the last block in picotez (e-12) per gas
            virtual Future<std::shared_ptr<BigInt>>
            getGasPrice() const = 0;

            virtual FuturePtr<TransactionsBulk>
            getTransactions(const std::string& address,
                            const Either<std::string, uint32_t>& token = {}) const = 0;

            virtual FuturePtr<Block>
            getCurrentBlock() const = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getEstimatedGasLimit(const std::string &address) const = 0;

            virtual Future<std::shared_ptr<GasLimit>>
            getEstimatedGasLimit(const std::shared_ptr<TezosLikeTransactionApi> &tx) const = 0;

            Future<std::shared_ptr<GasLimit>> getEstimatedGasLimit(
                const std::shared_ptr<HttpClient> &http,
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<TezosLikeTransactionApi> &transaction) const;

            Future<std::shared_ptr<GasLimit>> getEstimatedGasLimit(
                const std::shared_ptr<HttpClient> &http,
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<TezosLikeTransactionApi> &transaction,
                const std::string &chainId) const;


            Future<std::string> getChainId(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http) const;


            virtual Future<std::shared_ptr<BigInt>>
            getStorage(const std::string &address) const = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getCounter(const std::string &address) const = 0;

            virtual Future<std::vector<uint8_t>> forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx) const = 0;
            // This a helper to manage legacy KT accounts
            // WARNING: we will only support removing delegation and transfer from KT to implicit account
            static Future<std::vector<uint8_t>> forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx,
                                                                 const std::shared_ptr<api::ExecutionContext> &context,
                                                                 const std::shared_ptr<HttpClient> &http,
                                                                 const std::string &rpcNode);

            virtual Future<std::string> getManagerKey(const std::string &address) const = 0;
            // This a helper to manage legacy KT accounts
            // WARNING: we will only support removing delegation and transfer from KT to implicit account
            static Future<std::string> getManagerKey(const std::string &address,
                                                     const std::shared_ptr<api::ExecutionContext> &context,
                                                     const std::shared_ptr<HttpClient> &http,
                                                     const std::string &rpcNode);

            virtual Future<bool> isAllocated(const std::string &address) const = 0;
            static Future<bool> isAllocated(const std::string &address,
                                            const std::shared_ptr<api::ExecutionContext> &context,
                                            const std::shared_ptr<HttpClient> &http,
                                            const std::string &rpcNode);

            virtual Future<std::string> getCurrentDelegate(const std::string &address) const = 0;
            static Future<std::string> getCurrentDelegate(const std::string &address,
                                                          const std::shared_ptr<api::ExecutionContext> &context,
                                                          const std::shared_ptr<HttpClient> &http,
                                                          const std::string &rpcNode);

            /// Check that the account is funded.
            virtual Future<bool> isFunded(const std::string &address) const = 0;

            virtual Future<bool> isDelegate(const std::string &address) const = 0;

            /// Get a token balance for an account
            virtual Future<std::shared_ptr<BigInt>>
            getTokenBalance(const std::string& accountAddress,
                            const std::string& tokenAddress) const = 0;

        protected:
            inline const std::string& getRPCNodeEndpoint() const {
                return _rpcNode;
            }

            inline api::TezosLikeNetworkParameters getNetworkParameters() const override {
                return _parameters;
            }

            inline std::shared_ptr<api::ExecutionContext> getExplorerContext() const override {
                return getContext();
            }

        private:
            std::string _rpcNode;
            api::TezosLikeNetworkParameters _parameters;
        };

    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORER_H
