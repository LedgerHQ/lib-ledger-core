/*
 *
 * BitcoinLikeBlockchainExplorer
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/01/2017.
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
#ifndef LEDGER_CORE_BITCOINLIKEBLOCKCHAINEXPLORER_HPP
#define LEDGER_CORE_BITCOINLIKEBLOCKCHAINEXPLORER_HPP

#include <string>
#include <chrono>
#include <vector>
#include <utils/ConfigurationMatchable.h>
#include "../../../utils/optional.hpp"
#include "../../../api/ErrorCode.hpp"
#include "../../../utils/Option.hpp"
#include "../../../async/Future.hpp"
#include "../../../collections/collections.hpp"
#include "../../../math/BigInt.h"
#include <wallet/common/Block.h>

namespace ledger {
    namespace core {

        class BitcoinLikeBlockchainExplorer : public ConfigurationMatchable {
        public:
            typedef ledger::core::Block Block;

            struct Input {
                uint64_t index;
                Option<BigInt> value;
                Option<std::string> previousTxHash;
                Option<uint32_t> previousTxOutputIndex;
                Option<std::string> address;
                Option<std::string> signatureScript;
                Option<std::string> coinbase;
                uint32_t sequence;
                Input() {
                    sequence = 0xFFFFFFFF;
                };
            };

            struct Output {
                uint64_t index;
                std::string transactionHash;
                BigInt value;
                Option<std::string> address;
                std::string script;
                Output() = default;
                std::string time;
            };

            struct Transaction {
                uint32_t  version;
                std::string hash;
                std::chrono::system_clock::time_point receivedAt;
                uint64_t lockTime;
                Option<Block> block;
                std::vector<Input> inputs;
                std::vector<Output> outputs;
                Option<BigInt> fees;
                uint64_t confirmations;
                Transaction() {
                    version = 1;
                    confirmations = -1;
                }
            };

            struct TransactionsBulk {
                std::vector<Transaction> transactions;
                bool hasNext;
            };


        public:
            BitcoinLikeBlockchainExplorer(const std::shared_ptr<api::DynamicObject>& configuration, const std::vector<std::string> &matchableKeys);
            virtual Future<void *> startSession() = 0;
            virtual Future<Unit> killSession(void *session) = 0;

            virtual FuturePtr<TransactionsBulk> getTransactions(
                    const std::vector<std::string>& addresses,
                    Option<std::string> fromBlockHash = Option<std::string>(),
                    Option<void*> session = Option<void *>()
            ) = 0;

            virtual FuturePtr<Block> getCurrentBlock() = 0;
            virtual Future<Bytes> getRawTransaction(const String& transactionHash) = 0;
            virtual FuturePtr<Transaction> getTransactionByHash(const String& transactionHash) = 0;
            virtual Future<String> pushTransaction(const std::vector<uint8_t>& transaction) = 0;
            virtual Future<int64_t> getTimestamp() = 0;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEBLOCKCHAINEXPLORER_HPP
