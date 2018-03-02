/*
 *
 * BitcoinLikeTransactionBuilder.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 16/10/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#ifndef LEDGER_CORE_BITCOINLIKETRANSACTIONBUILDER_H
#define LEDGER_CORE_BITCOINLIKETRANSACTIONBUILDER_H

#include <vector>
#include <memory>
#include <api/BitcoinLikeOutput.hpp>
#include <soci.h>

namespace ledger {
    namespace core {
        class BitcoinLikeTransactionCreationRequest;
        class BitcoinLikeTransactionBuilder {
        public:
            virtual void buildTransaction(
                    soci::session& sql,
                    const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& inputs,
                    const std::vector<std::vector<uint8_t>>& signatures,
                    const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& outputs,
                    uint32_t lockTime,
                    std::vector<uint8_t>& rawTransaction
            ) = 0;
            virtual void estimateSize(const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& inputs,
                                      const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& outputs,
                                      uint64_t& out
            ) = 0;
            virtual void pickUTXO(soci::session& sql, const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& utxo,
                          std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& pickedUtxo) = 0;
        };
    }
}

#endif //LEDGER_CORE_BITCOINLIKETRANSACTIONBUILDER_H
