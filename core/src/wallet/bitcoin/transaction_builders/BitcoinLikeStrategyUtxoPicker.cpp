/*
 *
 * BitcoinLikeStrategyUtxoPicker.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/03/2018.
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

#include "BitcoinLikeStrategyUtxoPicker.h"
#include <api/BitcoinLikeScript.hpp>
#include <api/BitcoinLikeScriptChunk.hpp>
#include <wallet/bitcoin/api_impl/BitcoinLikeScriptApi.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>

namespace ledger {
    namespace core {

        BitcoinLikeStrategyUtxoPicker::BitcoinLikeStrategyUtxoPicker(const std::shared_ptr<api::ExecutionContext> &context,
                                                                     const api::Currency &currency) : BitcoinLikeUtxoPicker(context, currency) {

        }

        Future<BitcoinLikeUtxoPicker::UTXODescriptorList>
        BitcoinLikeStrategyUtxoPicker::filterInputs(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy) {
            return computeAggregatedAmount(buddy).flatMap<UTXODescriptorList>(ImmediateExecutionContext::INSTANCE, [=] (const BigInt& a) -> Future<UTXODescriptorList> {
                return buddy->getUtxo().map<UTXODescriptorList>(ImmediateExecutionContext::INSTANCE, [=] (const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& utxo) {
                    UTXODescriptorList picked;
                    BigInt aggregatedAmount = a;
                    auto picker = buddy->request.utxoPicker.getValue();
                    const api::BitcoinLikePickingStrategy strategy = std::get<0>(picker);
                    switch (strategy) {
                        case api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST:
                            filterWithDeepFirst(buddy, utxo, aggregatedAmount, picked);
                            break;
                        case api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE:
                            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Picking strategy OPTIMIZE_SIZE is not implemented");
                        case api::BitcoinLikePickingStrategy::MERGE_OUTPUTS:
                            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Picking strategy MERGE_OUTPUTS is not implemented");
                    }
                    return picked;
                });
            });
        }

        void
        BitcoinLikeStrategyUtxoPicker::filterWithDeepFirst(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                           const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxo,
                                                           BigInt &aggregatedAmount,
                                                           BitcoinLikeUtxoPicker::UTXODescriptorList &out) {

        }

        Future<BigInt> BitcoinLikeStrategyUtxoPicker::computeAggregatedAmount(
                const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy) {
            using UDList = std::list<BitcoinLikeUtxoPicker::UTXODescriptor>;
            std::function<Future<BigInt> (UDList::const_iterator, BigInt)> go;
            go = [=] (const UDList::const_iterator it, BigInt v) mutable -> Future<BigInt> {
                if (it == buddy->request.inputs.end())
                    return Future<BigInt>::successful(v);
                const auto& i = *it;
                const auto outputIndex = std::get<1>(i);
                return buddy->explorer->getTransactionByHash(String(std::get<0>(i))).flatMap<BigInt>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::Transaction>& tx) -> Future<BigInt> {
                    auto newIt = it;
                    return go(newIt++, v + tx->outputs[outputIndex].value);
                });
            };
            return go(buddy->request.inputs.begin(), BigInt());
        }

    }
}

