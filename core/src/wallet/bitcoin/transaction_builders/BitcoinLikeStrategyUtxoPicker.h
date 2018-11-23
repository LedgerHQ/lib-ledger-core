/*
 *
 * BitcoinLikeStrategyUtxoPicker.h
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

#pragma once

#include <wallet/bitcoin/transaction_builders/BitcoinLikeUtxoPicker.h>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class BitcoinLikeStrategyUtxoPicker : public BitcoinLikeUtxoPicker {
            public:
                BitcoinLikeStrategyUtxoPicker(const std::shared_ptr<api::ExecutionContext> &context,
                    const api::Currency &currency);
            protected:
                Future<UTXODescriptorList> filterInputs(const std::shared_ptr<Buddy> &buddy) override;
                Future<UTXODescriptorList> filterWithDeepFirst(const std::shared_ptr<Buddy> &buddy,
                    const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& utxo,
                    const BigInt& aggregatedAmount);
                bool hasEnough(const std::shared_ptr<Buddy>& buddy, const BigInt& aggregatedAmount, int inputCount, bool computeOutputAmount = false);
                inline Future<BigInt> computeAggregatedAmount(const std::shared_ptr<Buddy>& buddy);

                //Only usefull for filterWithLowestFees
                struct EffectiveUTXO {
                    std::shared_ptr<api::BitcoinLikeOutput> output;
                    int64_t effectiveValue;
                    int64_t effectiveFees;
                    int64_t longTermFees;
                };
                Future<BitcoinLikeUtxoPicker::UTXODescriptorList> filterWithOptimizeSize(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                    const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxos,
                    const BigInt &aggregatedAmount);

                Future<BitcoinLikeUtxoPicker::UTXODescriptorList> filterWithMergeOutputs(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                    const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxos,
                    const BigInt &aggregatedAmount);

                //Usefull for filterWithLowFeesFirst
                static const int64_t DEFAULT_FALLBACK_FEE = 20;
                static const int64_t DEFAULT_DISCARD_FEE = 10;
                static const int64_t COIN = 100000000;
                static const int64_t MAX_MONEY = 21000000 * COIN;
                static const uint32_t TOTAL_TRIES = 100000;
                static const int64_t CENT = 1000000;
                static const int64_t MIN_CHANGE = 100000;
            private:
                Future<BitcoinLikeUtxoPicker::UTXODescriptorList> filterWithKnapsackSolver(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                    const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxos,
                    const BigInt &aggregatedAmount);

            };
        }
    }
}
