/*
 *
 * BitcoinLikeUTXOPicker.h
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

#pragma once

#include <core/api/Currency.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/async/Future.hpp>

#include <bitcoin/BitcoinTypes.hpp>
#include <bitcoin/api/BitcoinLikeOutput.hpp>
#include <bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <bitcoin/transactions/BitcoinLikeTransactionBuilder.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeTransaction;
        class BitcoinLikeWritableInput;
        using BitcoinLikeGetUTXOFunction = std::function<Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> ()>;
        using BitcoinLikeGetTxFunction = std::function<FuturePtr<BitcoinLikeBlockchainExplorerTransaction> (const std::string&)>;

        class BitcoinLikeUTXOPicker : public DedicatedContext, public std::enable_shared_from_this<BitcoinLikeUTXOPicker> {
        public:
            BitcoinLikeUTXOPicker(
                    const std::shared_ptr<api::ExecutionContext> &context,
                    const api::Currency& currency
            );
            virtual BitcoinLikeTransactionBuildFunction getBuildFunction(
                    const BitcoinLikeGetUTXOFunction& getUTXO,
                    const BitcoinLikeGetTxFunction& getTransaction,
                    const std::shared_ptr<BitcoinLikeBlockchainExplorer>& explorer,
                    const std::shared_ptr<BitcoinLikeKeychain>& keychain,
                    const uint64_t currentBlockHeight,
                    const std::shared_ptr<spdlog::logger>& logger,
                    bool partial);
            const api::Currency& getCurrency() const;

        protected:
            using UTXODescriptor = std::tuple<std::string, int32_t, uint32_t >;
            using UTXODescriptorList = std::vector<UTXODescriptor>;
            struct Buddy {
                Buddy(
                        const BitcoinLikeTransactionBuildRequest& r,
                        const BitcoinLikeGetUTXOFunction& g,
                        const BitcoinLikeGetTxFunction& tx,
                        const std::shared_ptr<BitcoinLikeBlockchainExplorer>& e,
                        const std::shared_ptr<BitcoinLikeKeychain>& k,
                        const std::shared_ptr<spdlog::logger>& l,
                        std::shared_ptr<BitcoinLikeTransaction> t,
                        bool partial) : request(r), explorer(e), keychain(k), transaction(t), getUTXO(g),
                          getTransaction(tx), logger(l), isPartial(partial)
                {
                    if(request.wipe) {
                        outputAmount = ledger::core::BigInt::ZERO;
                    } else {
                        for (auto& output : r.outputs)
                            outputAmount = outputAmount + *std::get<0>(output);
                    }
                }
                const BitcoinLikeTransactionBuildRequest request;
                BitcoinLikeGetUTXOFunction getUTXO;
                BitcoinLikeGetTxFunction getTransaction;
                std::shared_ptr<BitcoinLikeBlockchainExplorer> explorer;
                std::shared_ptr<BitcoinLikeKeychain> keychain;
                std::shared_ptr<BitcoinLikeTransaction> transaction;
                BigInt outputAmount;
                std::shared_ptr<spdlog::logger> logger;
                BigInt changeAmount;
                bool isPartial;
            };

            virtual Future<Unit> fillInputs(const std::shared_ptr<Buddy>& buddy);
            virtual Future<UTXODescriptorList> filterInputs(const std::shared_ptr<Buddy>& buddy) = 0;
            virtual Future<Unit> fillOutputs(const std::shared_ptr<Buddy>& buddy);
            virtual Future<Unit> fillTransactionInfo(const std::shared_ptr<Buddy>& buddy);

        private:
            Future<Unit> fillInput(const std::shared_ptr<Buddy>& buddy, const UTXODescriptor& desc);
            BitcoinLikeGetUTXOFunction createFilteredUTXOFunction(const BitcoinLikeTransactionBuildRequest& buddy,
                                                                  const BitcoinLikeGetUTXOFunction& getUTXO);
        private:
            api::Currency _currency;
        };
    }
}