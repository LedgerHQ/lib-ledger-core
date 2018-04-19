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
                buddy->logger->info("GET UTXO");
                return buddy->getUtxo().flatMap<UTXODescriptorList>(ImmediateExecutionContext::INSTANCE, [=] (const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& utxo) -> Future<UTXODescriptorList> {
                    buddy->logger->info("GOT UTXO");
                    if (utxo.size() == 0)
                        throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "There is no UTXO on this account.");
                    auto picker = buddy->request.utxoPicker.getValue();
                    const api::BitcoinLikePickingStrategy strategy = std::get<0>(picker);
                    switch (strategy) {
                        case api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST:
                            return filterWithDeepFirst(buddy, utxo, a);
                        case api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE:
                            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Picking strategy OPTIMIZE_SIZE is not implemented");
                        case api::BitcoinLikePickingStrategy::MERGE_OUTPUTS:
                            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Picking strategy MERGE_OUTPUTS is not implemented");
                    }
                });
            });
        }

        Future<BigInt> BitcoinLikeStrategyUtxoPicker::computeAggregatedAmount(
                const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy) {
            using UDList = std::list<BitcoinLikeUtxoPicker::UTXODescriptor>;
            static std::function<Future<BigInt> (UDList::const_iterator, BigInt, const std::shared_ptr<Buddy>& buddy)> go
                    = [] (const UDList::const_iterator it, BigInt v, const std::shared_ptr<Buddy>& buddy) mutable -> Future<BigInt> {
                if (it == buddy->request.inputs.end())
                    return Future<BigInt>::successful(v);
                const auto& i = *it;
                const auto outputIndex = std::get<1>(i);
                buddy->logger->info("GET TX 1");
                return buddy->explorer->getTransactionByHash(String(std::get<0>(i))).flatMap<BigInt>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::Transaction>& tx) -> Future<BigInt> {
                    buddy->logger->info("GOT TX 1");
                    auto newIt = it;
                    return go(newIt++, v + tx->outputs[outputIndex].value, buddy);
                });
            };
            return go(buddy->request.inputs.begin(), BigInt(), buddy);
        }

        Future<BitcoinLikeUtxoPicker::UTXODescriptorList>
        BitcoinLikeStrategyUtxoPicker::filterWithDeepFirst(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                           const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxo,
                                                           const BigInt &aggregatedAmount) {
            // First add block height and time information to the list of utxo
            using RichUTXO = std::tuple<uint64_t, std::shared_ptr<api::BitcoinLikeOutput>>;
            using RichUTXOList = std::vector<RichUTXO>;
            std::shared_ptr<RichUTXOList> richutxo = std::make_shared<RichUTXOList>();

            static  Future<Unit> (*go)(int index, const std::shared_ptr<Buddy>& buddy, const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxo, std::shared_ptr<RichUTXOList>)
            = [] (int index, const std::shared_ptr<Buddy>& buddy, const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxo, std::shared_ptr<RichUTXOList> richutxo) -> Future<Unit> {
                if (index >= utxo.size()) {
                    return Future<Unit>::successful(unit);
                }
                auto hash = utxo[index]->getTransactionHash();
                buddy->logger->info("GET TX 2");
                return buddy->getTransaction(hash).flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::Transaction>& tx) mutable -> Future<Unit> {
                    buddy->logger->info("GOT TX 2");
                    if (tx->block.nonEmpty())
                        richutxo->push_back(RichUTXO({tx->block.getValue().height, utxo[index]}));
                    else
                        richutxo->push_back(RichUTXO({std::numeric_limits<uint64_t>::max(), utxo[index]}));
                    buddy->logger->info("Go {} on {}", index + 1, utxo.size());
                    return go(index + 1, buddy, utxo, richutxo);
                });
            };
            return go(0, buddy, utxo, richutxo).map<UTXODescriptorList>(ImmediateExecutionContext::INSTANCE, [=] (const Unit& u) -> UTXODescriptorList {
                // Sort the list by deep
                std::sort(richutxo->begin(), richutxo->end(), [] (const RichUTXO& a, const RichUTXO& b) -> bool {
                    return std::get<0>(a) < std::get<0>(b);
                });
                // Aggregate the amount needed by the transaction
                BigInt amount = aggregatedAmount;
                auto pickedInputs = 0;
                for (const RichUTXO & ru : (*richutxo)) {
                    const std::shared_ptr<BitcoinLikeOutputApi>& output = std::dynamic_pointer_cast<BitcoinLikeOutputApi>(std::get<1>(ru));
                    amount = amount + output->value();
                    buddy->logger->debug("Collected: {} Needed: {}", amount.toString(), buddy->outputAmount.toString());
                    pickedInputs += 1;

                    if (hasEnough(buddy, amount, pickedInputs))
                        break;
                    if (pickedInputs >= richutxo->size())
                        throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "Cannot gather enough funds.");
                }

                buddy->logger->debug("Require {} inputs to complete the transaction with {} for {}", pickedInputs, amount.toString(), buddy->outputAmount.toString());

                // Build UTXODescriptorList from our RichUTXOList
                UTXODescriptorList out;

                for (auto index = 0; index < pickedInputs; index++) {
                    const std::shared_ptr<BitcoinLikeOutputApi>& output =
                            std::dynamic_pointer_cast<BitcoinLikeOutputApi>(std::get<1>((*richutxo)[index]));
                    out.push_back(UTXODescriptor({output->getTransactionHash(), output->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())}));
                }

                return out;
            });
        }

        bool BitcoinLikeStrategyUtxoPicker::hasEnough(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                      const BigInt &aggregatedAmount,
                                                      int inputCount) {
            if (buddy->outputAmount > aggregatedAmount) return false;
            // TODO Handle multiple outputs
            // TODO Handle segwit

            auto computeAmountWithFees = [&] (int addedOutputCount) -> BigInt {
                auto size = BitcoinLikeTransactionApi::estimateSize(
                        inputCount,
                        buddy->request.outputs.size() + addedOutputCount,
                        getCurrency().bitcoinLikeNetworkParameters.value().UsesTimestampedTransaction, false
                );
                buddy->logger->debug("Estimate for {} inputs with {} outputs", inputCount,  buddy->request.outputs.size() + addedOutputCount);
                buddy->logger->debug("Estimated size {} <> {}", size.min, size.max);
                return buddy->outputAmount + (*buddy->request.feePerByte * BigInt(size.max));
            };

            // Check the amount of fees needed when we don't need to add a change output to the transaction
            auto minimumNeededAmount = computeAmountWithFees(0);
            buddy->logger->debug("Minimum required with fees {} got {}", minimumNeededAmount.toString(), aggregatedAmount.toString());
            if (buddy->outputAmount > minimumNeededAmount) return false;
            BigInt changeAmount = aggregatedAmount - minimumNeededAmount;
            buddy->changeAmount = changeAmount;
            buddy->logger->debug("Change amount {}, dust {}", changeAmount.toString(), BigInt(getCurrency().bitcoinLikeNetworkParameters.value().DustAmount).toString());
            if (changeAmount > BigInt(getCurrency().bitcoinLikeNetworkParameters.value().DustAmount)) {
                // The change amount is bigger than the dust so we need to create a change output,
                // let's see if we have enough fees to handle this output
                auto minimumNeededAmountWithChange = computeAmountWithFees(1);
                buddy->logger->debug("Minimum required with change {} got {}", minimumNeededAmountWithChange.toString(), aggregatedAmount.toString());
                if (buddy->outputAmount > minimumNeededAmountWithChange) return false;
            }
            return true;
        }

    }
}

