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

#ifndef LEDGER_CORE_BITCOINLIKESTRATEGYUTXOPICKER_H
#define LEDGER_CORE_BITCOINLIKESTRATEGYUTXOPICKER_H

#include "BitcoinLikeUtxoPicker.h"
#include <wallet/bitcoin/transaction_builders/BitcoinLikeUtxo.hpp>

namespace ledger {
    namespace core {

        namespace api {
            class Currency;
        };

        class BitcoinLikeStrategyUtxoPicker : public BitcoinLikeUtxoPicker {
        public:
            BitcoinLikeStrategyUtxoPicker(const std::shared_ptr<api::ExecutionContext> &context,
                                          const api::Currency &currency);
        public:
            static std::vector<BitcoinLikeUtxo> filterWithKnapsackSolver(const std::shared_ptr<Buddy>& buddy,
                const std::vector<BitcoinLikeUtxo>& utxos,
                const BigInt& aggregatedAmount,
                const api::Currency& currrency);

            static std::vector<BitcoinLikeUtxo> filterWithOptimizeSize(const std::shared_ptr<Buddy>& buddy,
                const std::vector<BitcoinLikeUtxo>& utxos,
                const BigInt& aggregatedAmount,
                const api::Currency& currrency);

            static std::vector<BitcoinLikeUtxo> filterWithMergeOutputs(const std::shared_ptr<Buddy>& buddy,
                const std::vector<BitcoinLikeUtxo>& utxos,
                const BigInt& aggregatedAmount,
                const api::Currency& currrency);
            static std::vector<BitcoinLikeUtxo> filterWithDeepFirst(const std::shared_ptr<Buddy>& buddy,
                const std::vector<BitcoinLikeUtxo>& utxo,
                const BigInt& aggregatedAmount,
                const api::Currency& currrency);
            static bool hasEnough(const std::shared_ptr<Buddy>& buddy,
                const BigInt& aggregatedAmount,
                int inputCount,
                const api::Currency& currrency,
                bool computeOutputAmount);
        protected:
            Future<std::vector<BitcoinLikeUtxo>> filterInputs(const std::shared_ptr<Buddy> &buddy) override;

            inline Future<BigInt> computeAggregatedAmount(const std::shared_ptr<Buddy>& buddy);

            //Usefull for filterWithLowFeesFirst
            static const int64_t DEFAULT_FALLBACK_FEE = 20;
            static const int64_t DEFAULT_DISCARD_FEE = 10;
            static const int64_t COIN = 100000000;
            static const int64_t MAX_MONEY = 21000000 * COIN;
            static const uint32_t TOTAL_TRIES = 10000;
            static const int64_t CENT = 1000000;
        private:

            static std::vector<BitcoinLikeUtxo> filterWithSort(
                const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                std::vector<BitcoinLikeUtxo> utxos,
                BigInt amount,
                const api::Currency &currency,
                std::function<bool(BitcoinLikeUtxo&, BitcoinLikeUtxo&)> const& functor
            );
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKESTRATEGYUTXOPICKER_H
