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
#include <collections/DynamicObject.hpp>
#include <math/BigInt.h>
#include <net/HttpClient.hpp>
#include <string>
#include <utils/ConfigurationMatchable.h>
#include <utils/Option.hpp>
#include <wallet/common/Block.h>
#include <wallet/common/explorers/AbstractBlockchainExplorer.h>
#include <wallet/tezos/keychains/TezosLikeKeychain.h>

namespace ledger {
    namespace core {
        namespace api {
            class OperationQuery;
        }

        struct TezosLikeBlockchainExplorerOriginatedAccount {
            TezosLikeBlockchainExplorerOriginatedAccount(const std::string &a = "",
                                                         bool isSpendable     = false,
                                                         bool isDelegatable   = false) : address(a),
                                                                                       spendable(isSpendable),
                                                                                       delegatable(isDelegatable){};

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
            int64_t counter{0};
            Option<std::string> explorerId; // For BakingBad only

            TezosLikeBlockchainExplorerTransaction() {
                confirmations = 0;
                type          = api::TezosOperationTag::OPERATION_TAG_NONE;
                status        = 0;
            }

            TezosLikeBlockchainExplorerTransaction(const TezosLikeBlockchainExplorerTransaction &cpy) = default;
        };

        struct GasLimit {
            BigInt reveal;
            BigInt transaction;

            GasLimit() : reveal(0), transaction(0) {}

            GasLimit(const BigInt &r, const BigInt &t) : reveal(r), transaction(t) {}
        };

        class TezosLikeTransactionApi;
        class TezosLikeBlockchainExplorer : public ConfigurationMatchable,
                                            public AbstractBlockchainExplorer<TezosLikeBlockchainExplorerTransaction> {
          public:
            typedef ledger::core::Block Block;
            using Transaction = TezosLikeBlockchainExplorerTransaction;

            TezosLikeBlockchainExplorer(const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
                                        const std::vector<std::string> &matchableKeys);

            virtual Future<std::shared_ptr<BigInt>>
            getBalance(const std::vector<TezosLikeKeychain::Address> &addresses) = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getFees() = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getEstimatedGasLimit(const std::string &address) = 0;

            virtual Future<std::shared_ptr<GasLimit>> getEstimatedGasLimit(
                const std::shared_ptr<TezosLikeTransactionApi> &tx) = 0;

            Future<std::shared_ptr<GasLimit>> getEstimatedGasLimit(
                const std::shared_ptr<HttpClient> &http,
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<TezosLikeTransactionApi> &transaction);

            Future<std::shared_ptr<GasLimit>> getEstimatedGasLimit(
                const std::shared_ptr<HttpClient> &http,
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<TezosLikeTransactionApi> &transaction,
                const std::string &chainId);

            Future<std::string> getChainId(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<HttpClient> &http);

            virtual Future<std::shared_ptr<BigInt>>
            getStorage(const std::string &address) = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getCounter(const std::string &address) = 0;

            virtual Future<std::vector<uint8_t>> forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx) = 0;
            // This a helper to manage legacy KT accounts
            // WARNING: we will only support removing delegation and transfer from KT to implicit account
            static Future<std::vector<uint8_t>> forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx,
                                                                 const std::shared_ptr<api::ExecutionContext> &context,
                                                                 const std::shared_ptr<HttpClient> &http,
                                                                 const std::string &rpcNode);

            virtual Future<std::string> getManagerKey(const std::string &address) = 0;
            // This a helper to manage legacy KT accounts
            // WARNING: we will only support removing delegation and transfer from KT to implicit account
            static Future<std::string> getManagerKey(const std::string &address,
                                                     const std::shared_ptr<api::ExecutionContext> &context,
                                                     const std::shared_ptr<HttpClient> &http,
                                                     const std::string &rpcNode);

            virtual Future<bool> isAllocated(const std::string &address) = 0;
            static Future<bool> isAllocated(const std::string &address,
                                            const std::shared_ptr<api::ExecutionContext> &context,
                                            const std::shared_ptr<HttpClient> &http,
                                            const std::string &rpcNode);

            virtual Future<std::string> getCurrentDelegate(const std::string &address) = 0;
            static Future<std::string> getCurrentDelegate(const std::string &address,
                                                          const std::shared_ptr<api::ExecutionContext> &context,
                                                          const std::shared_ptr<HttpClient> &http,
                                                          const std::string &rpcNode);

            /// Check that the account is funded.
            virtual Future<bool> isFunded(const std::string &address) = 0;

            virtual Future<bool> isDelegate(const std::string &address) = 0;

            virtual Future<std::string> getSynchronisationOffset(const std::shared_ptr<api::OperationQuery> &operations) = 0;

          protected:
            std::string getRPCNodeEndpoint() const {
                return _rpcNode;
            };

          private:
            std::string _rpcNode;
        };
    } // namespace core
} // namespace ledger
#endif // LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORER_H
