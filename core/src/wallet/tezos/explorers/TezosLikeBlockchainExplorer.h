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

#include <string>

#include <api/DynamicObject.hpp>
#include <api/ExecutionContext.hpp>
#include <api/TezosLikeNetworkParameters.hpp>
#include <async/DedicatedContext.hpp>
#include <collections/DynamicObject.hpp>
#include <math/BigInt.h>
#include <net/HttpClient.hpp>
#include <utils/ConfigurationMatchable.h>
#include <utils/Option.hpp>
#include <wallet/common/Block.h>
#include <wallet/common/explorers/AbstractBlockchainExplorer.h>
#include <wallet/tezos/keychains/TezosLikeKeychain.h>
#include <api/TezosOperationTag.hpp>

namespace ledger {
    namespace core {

        struct TezosLikeBlockchainExplorerOriginatedAccount {
            TezosLikeBlockchainExplorerOriginatedAccount(const std::string &a = "",
                                                         bool isSpendable = false,
                                                         bool isDelegatable = false) :
                    address(a),
                    spendable(isSpendable),
                    delegatable(isDelegatable) {
            };

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
            TezosLikeBlockchainExplorerTransaction() {
                confirmations = 0;
                type = api::TezosOperationTag::OPERATION_TAG_NONE;
            }

            TezosLikeBlockchainExplorerTransaction(const TezosLikeBlockchainExplorerTransaction &cpy) {
                this->hash = cpy.hash;
                this->receivedAt = cpy.receivedAt;
                this->value = cpy.value;
                this->fees = cpy.fees;
                this->gas_limit = cpy.gas_limit;
                this->storage_limit = cpy.storage_limit;
                this->receiver = cpy.receiver;
                this->sender = cpy.sender;
                this->block = cpy.block;
                this->confirmations = cpy.confirmations;
                this->type = cpy.type;
                this->publicKey = cpy.publicKey;
                this->originatedAccount = cpy.originatedAccount;
            }

        };
        class TezosLikeTransactionApi;
        class TezosLikeBlockchainExplorer : public ConfigurationMatchable,
                                            public AbstractBlockchainExplorer<TezosLikeBlockchainExplorerTransaction> {
        public:
            typedef ledger::core::Block Block;

            TezosLikeBlockchainExplorer(const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
                                        const std::vector<std::string> &matchableKeys);

            virtual Future<std::shared_ptr<BigInt>>
            getBalance(const std::vector<TezosLikeKeychain::Address> &addresses) = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getFees() = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getEstimatedGasLimit(const std::string &address) = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getStorage(const std::string &address) = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getCounter(const std::string &address) = 0;

            virtual Future<std::vector<uint8_t>> forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx) = 0;
            // This a helper to manage legacy KT accounts
            // WARNING: we will only support removing delegation and transfer from KT to implicit account
            static Future<std::vector<uint8_t>> forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx,
                                                                 const std::shared_ptr<api::ExecutionContext> &context,
                                                                 const std::shared_ptr<HttpClient> &http);
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORER_H
