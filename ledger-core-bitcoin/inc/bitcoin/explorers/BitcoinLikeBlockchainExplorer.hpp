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

#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <core/api/BigInt.hpp>
#include <core/api/Error.hpp>
#include <core/async/Future.hpp>
#include <core/collections/Collections.hpp>
#include <core/explorers/AbstractBlockchainExplorer.hpp>
#include <core/math/BigInt.hpp>
#include <core/utils/ConfigurationMatchable.hpp>
#include <core/utils/optional.hpp>
#include <core/utils/Option.hpp>

namespace ledger {
    namespace core {
        struct BitcoinLikeBlockchainExplorerInput {
            uint64_t index;
            Option<BigInt> value;
            Option<std::string> previousTxHash;
            Option<uint32_t> previousTxOutputIndex;
            Option<std::string> address;
            Option<std::string> signatureScript;
            Option<std::string> coinbase;
            uint32_t sequence;

            BitcoinLikeBlockchainExplorerInput() {
                sequence = 0xFFFFFFFF;
            }
        };

        struct BitcoinLikeBlockchainExplorerOutput {
            uint64_t index;
            std::string transactionHash;
            BigInt value;
            Option<std::string> address;
            std::string script;
            std::string time;
            Option<uint64_t> blockHeight;

            BitcoinLikeBlockchainExplorerOutput() = default;
        };

        struct BitcoinLikeBlockchainExplorerTransaction {
            uint32_t  version;
            std::string hash;
            std::chrono::system_clock::time_point receivedAt;
            uint64_t lockTime;
            Option<api::Block> block;
            std::vector<BitcoinLikeBlockchainExplorerInput> inputs;
            std::vector<BitcoinLikeBlockchainExplorerOutput> outputs;
            Option<BigInt> fees;
            uint64_t confirmations;

            BitcoinLikeBlockchainExplorerTransaction() {
                version = 1;
                confirmations = -1;
            }

            BitcoinLikeBlockchainExplorerTransaction(const BitcoinLikeBlockchainExplorerTransaction &cpy) {
                this->confirmations = cpy.confirmations;
                this->version = cpy.version;
                this->outputs = cpy.outputs;
                this->inputs = cpy.inputs;
                this->receivedAt = cpy.receivedAt;
                this->lockTime = cpy.lockTime;
                this->fees = cpy.fees;
                this->hash = cpy.hash;
                this->block = cpy.block;
            }
        };

        class BitcoinLikeBlockchainExplorer : public ConfigurationMatchable,
                                              public AbstractBlockchainExplorer<BitcoinLikeBlockchainExplorerTransaction> {
        public:
            using Block = api::Block;
            
            BitcoinLikeBlockchainExplorer(const std::shared_ptr<api::DynamicObject>& configuration,
                                          const std::vector<std::string> &matchableKeys);

            virtual Future<std::vector<std::shared_ptr<api::BigInt>>> getFees() = 0;

        };
    }
}
