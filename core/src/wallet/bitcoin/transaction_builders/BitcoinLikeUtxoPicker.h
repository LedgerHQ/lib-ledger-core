/*
 *
 * BitcoinLikeUtxoPicker.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/03/2018.
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

#ifndef LEDGER_CORE_BITCOINLIKEUTXOPICKER_H
#define LEDGER_CORE_BITCOINLIKEUTXOPICKER_H

#include <async/DedicatedContext.hpp>
#include "BitcoinLikeTransactionBuilder.h"
#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <wallet/bitcoin/types.h>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <api/Currency.hpp>
#include <async/Future.hpp>
#include <api/BitcoinLikeOutput.hpp>


namespace ledger {
    namespace core {
        class BitcoinLikeTransactionApi;
        class BitcoinLikeWritableInputApi;
        using BitcoinLikeGetUtxoFunction = std::function<Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> ()>;
        using BitcoinLikeGetTxFunction = std::function<FuturePtr<BitcoinLikeBlockchainExplorer::Transaction> (const std::string&)>;

        class BitcoinLikeUtxoPicker : public DedicatedContext, public std::enable_shared_from_this<BitcoinLikeUtxoPicker> {
        public:
            BitcoinLikeUtxoPicker(
                    const std::shared_ptr<api::ExecutionContext> &context,
                    const api::Currency& currency
            );
            virtual BitcoinLikeTransactionBuildFunction getBuildFunction(
                    const BitcoinLikeGetUtxoFunction& getUtxo,
                    const BitcoinLikeGetTxFunction& getTransaction,
                    const std::shared_ptr<BitcoinLikeBlockchainExplorer>& explorer,
                    const std::shared_ptr<BitcoinLikeKeychain>& keychain,
                    const uint64_t currentBlockHeight,
                    const std::shared_ptr<spdlog::logger>& logger
            );
            const api::Currency& getCurrency() const;

        protected:
            using UTXODescriptor = std::tuple<std::string, int32_t, uint32_t >;
            using UTXODescriptorList = std::vector<UTXODescriptor>;
            struct Buddy {
                Buddy(
                        const BitcoinLikeTransactionBuildRequest& r,
                        const BitcoinLikeGetUtxoFunction& g,
                        const BitcoinLikeGetTxFunction& tx,
                        const std::shared_ptr<BitcoinLikeBlockchainExplorer>& e,
                        const std::shared_ptr<BitcoinLikeKeychain>& k,
                        const std::shared_ptr<spdlog::logger>& l,
                        std::shared_ptr<BitcoinLikeTransactionApi> t)
                        : request(r), explorer(e), keychain(k), transaction(t), getUtxo(g),
                          getTransaction(tx), logger(l)
                {
                    if(request.wipe) {
                        outputAmount = ledger::core::BigInt::ZERO;
                    } else {
                        for (auto& output : r.outputs)
                            outputAmount = outputAmount + *std::get<0>(output);
                    }
                }
                const BitcoinLikeTransactionBuildRequest request;
                BitcoinLikeGetUtxoFunction getUtxo;
                BitcoinLikeGetTxFunction getTransaction;
                std::shared_ptr<BitcoinLikeBlockchainExplorer> explorer;
                std::shared_ptr<BitcoinLikeKeychain> keychain;
                std::shared_ptr<BitcoinLikeTransactionApi> transaction;
                BigInt outputAmount;
                std::shared_ptr<spdlog::logger> logger;
                BigInt changeAmount;
            };

            virtual Future<Unit> fillInputs(const std::shared_ptr<Buddy>& buddy);
            virtual Future<UTXODescriptorList> filterInputs(const std::shared_ptr<Buddy>& buddy) = 0;
            virtual Future<Unit> fillOutputs(const std::shared_ptr<Buddy>& buddy);
            virtual Future<Unit> fillTransactionInfo(const std::shared_ptr<Buddy>& buddy);

        private:
            Future<Unit> fillInput(const std::shared_ptr<Buddy>& buddy, const UTXODescriptor& desc);
            BitcoinLikeGetUtxoFunction createFilteredUtxoFunction(const BitcoinLikeTransactionBuildRequest& buddy,
                                                                  const BitcoinLikeGetUtxoFunction& getUtxo);
        private:
            api::Currency _currency;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEUTXOPICKER_H
