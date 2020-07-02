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
#include <random>
namespace ledger {
    namespace core {

        BitcoinLikeStrategyUtxoPicker::BitcoinLikeStrategyUtxoPicker(const std::shared_ptr<api::ExecutionContext> &context,
                                                                     const api::Currency &currency) : BitcoinLikeUtxoPicker(context, currency) {

        }

        Future<BitcoinLikeUtxoPicker::UTXODescriptorList>
        BitcoinLikeStrategyUtxoPicker::filterInputs(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy) {
            return computeAggregatedAmount(buddy).flatMap<UTXODescriptorList>(getContext(), [=] (const BigInt& a) -> Future<UTXODescriptorList> {
                buddy->logger->info("GET UTXO");
                return buddy->getUtxo().map<UTXODescriptorList>(getContext(), [=] (const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& utxo) -> UTXODescriptorList {
                    buddy->logger->info("GOT UTXO");
                    if (utxo.size() == 0)
                        throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "There is no UTXO on this account.");
                    //If wipe mode no matter which strategy we use, let's use filterWithDeepFirst (for the moment)
                    if (buddy->request.wipe) {
                        return filterWithDeepFirst(buddy, utxo, a, getCurrency());
                    }
                    auto picker = buddy->request.utxoPicker.getValue();
                    const api::BitcoinLikePickingStrategy strategy = std::get<0>(picker);
                    switch (strategy) {
                        case api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST:
                            return filterWithDeepFirst(buddy, utxo, a, getCurrency());
                        case api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE:
                            return filterWithOptimizeSize(buddy, utxo, a, getCurrency());
                        case api::BitcoinLikePickingStrategy::MERGE_OUTPUTS:
                            return filterWithMergeOutputs(buddy, utxo, a, getCurrency());
                    }
                });
            });
        }

        Future<BigInt> BitcoinLikeStrategyUtxoPicker::computeAggregatedAmount(
                const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy) {
            using UDList = std::list<BitcoinLikeUtxoPicker::UTXODescriptor>;
            static std::function<Future<BigInt> (std::shared_ptr<api::ExecutionContext>, UDList::const_iterator, BigInt, const std::shared_ptr<Buddy>& buddy)> go
                    = [] (std::shared_ptr<api::ExecutionContext> context, const UDList::const_iterator it, BigInt v, const std::shared_ptr<Buddy>& buddy) mutable -> Future<BigInt> {
                        if (it == buddy->request.inputs.end())
                            return Future<BigInt>::successful(v);
                        const auto& i = *it;
                        const auto outputIndex = std::get<1>(i);
                        buddy->logger->info("GET TX 1");
                        return buddy->getTransaction(String(std::get<0>(i))).flatMap<BigInt>(context, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorerTransaction>& tx) -> Future<BigInt> {
                            buddy->logger->info("GOT TX 1");
                            auto newIt = it;
                            return go(context, newIt++, v + tx->outputs[outputIndex].value, buddy);
                        });
            };
            return go(getContext(), buddy->request.inputs.begin(), BigInt(), buddy);
        }

        BitcoinLikeUtxoPicker::UTXODescriptorList
        BitcoinLikeStrategyUtxoPicker::filterWithDeepFirst(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                           const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxo,
                                                           const BigInt &aggregatedAmount,
                                                           const api::Currency& currency
        ) {
            // First add block height and time information to the list of utxo
            using RichUTXO = std::tuple<uint64_t, std::shared_ptr<api::BitcoinLikeOutput>>;
            using RichUTXOList = std::vector<RichUTXO>;
            std::shared_ptr<RichUTXOList> richutxo = std::make_shared<RichUTXOList>();

            for (auto &u : utxo) {
                uint64_t block_height = u->getBlockHeight().value_or(std::numeric_limits<uint64_t>::max());
                RichUTXO curr_richutxo{block_height, u};
                richutxo->emplace_back(std::move(curr_richutxo));
            }

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
                bool computeOutputAmount = pickedInputs == utxo.size();
                if (hasEnough(buddy, amount, pickedInputs, currency, computeOutputAmount)) {
                    break;
                }
                if (pickedInputs >= richutxo->size() && !buddy->request.wipe) {
                    throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "Cannot gather enough funds.");
                }
            }

            buddy->logger->debug("Require {} inputs to complete the transaction with {} for {}", pickedInputs, amount.toString(), buddy->outputAmount.toString());

            // Build UTXODescriptorList from our RichUTXOList
            UTXODescriptorList out;

            for (auto index = 0; index < pickedInputs; index++) {
                const std::shared_ptr<BitcoinLikeOutputApi>& output =
                        std::dynamic_pointer_cast<BitcoinLikeOutputApi>(std::get<1>((*richutxo)[index]));
                //Fix: use uniform initialization
                UTXODescriptor utxoDescriptor{output->getTransactionHash(), output->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())};
                out.emplace_back(std::move(utxoDescriptor));
            }

            return out;
        }

        bool BitcoinLikeStrategyUtxoPicker::hasEnough(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                      const BigInt &aggregatedAmount,
                                                      int inputCount,
                                                      const api::Currency& currency,
                                                      bool computeOutputAmount) {
            if (buddy->outputAmount > aggregatedAmount) return false;
            // TODO Handle multiple outputs

            auto computeAmountWithFees = [&] (int addedOutputCount) -> BigInt {
                auto outputCount = buddy->request.outputs.size() + addedOutputCount;
                auto size = BitcoinLikeTransactionApi::estimateSize(inputCount,
                                                                    outputCount,
                                                                    currency,
                                                                    buddy->keychain->getKeychainEngine());
                buddy->logger->debug("Estimate for {} inputs with {} outputs", inputCount,  buddy->request.outputs.size() + addedOutputCount);
                buddy->logger->debug("Estimated size {} <> {}", size.Min, size.Max);
                return buddy->outputAmount + (*buddy->request.feePerByte * BigInt(size.Max));
            };

            // Check the amount of fees needed when we don't need to add a change output to the transaction
            auto minimumNeededAmount = computeAmountWithFees(0);
            buddy->logger->debug("Minimum required with fees {} got {}", minimumNeededAmount.toString(), aggregatedAmount.toString());
            if (buddy->outputAmount > minimumNeededAmount || aggregatedAmount < minimumNeededAmount) return false;

            //No need for change if we're wiping
            if(!buddy->request.wipe) {
                BigInt changeAmount = aggregatedAmount - minimumNeededAmount;
                buddy->changeAmount = changeAmount;
                buddy->logger->debug("Change amount {}, dust {}", changeAmount.toString(), BigInt(currency.bitcoinLikeNetworkParameters.value().DustAmount).toString());
                if (changeAmount > BigInt(currency.bitcoinLikeNetworkParameters.value().DustAmount)) {
                    // The change amount is bigger than the dust so we need to create a change output,
                    // let's see if we have enough fees to handle this output
                    auto minimumNeededAmountWithChange = computeAmountWithFees(1);
                    buddy->changeAmount = aggregatedAmount - minimumNeededAmountWithChange;
                    buddy->logger->debug("Minimum required with change {} got {}", minimumNeededAmountWithChange.toString(), aggregatedAmount.toString());
                    if (buddy->outputAmount > minimumNeededAmountWithChange) return false;
                }
            } else if(computeOutputAmount) {
                buddy->outputAmount = aggregatedAmount - minimumNeededAmount;
            }
            return !buddy->request.wipe;
        }

        BitcoinLikeUtxoPicker::UTXODescriptorList
        BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                              const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxos,
                                                              const BigInt &aggregatedAmount,
                                                              const api::Currency& currency
        ) {

            //Don't use this strategy for wipe mode (we have more performent strategies for this use case)
            if (buddy->request.wipe) {
                buddy->logger->debug("Strategy filterWithOptimizeSize with wipe to address mode, using filterWithDeepFirst");
                return filterWithDeepFirst(buddy, utxos, buddy->outputAmount, currency);
            }
            /*
             * This coin selection is inspired from the one used in Bitcoin Core
             * for more details please refer to SelectCoinsBnB
             * https://github.com/bitcoin/bitcoin/blob/0c5f67b8e5d9a502c6d321c5e0696bc3e9b4690d/src/wallet/coinselection.cpp
             * A coin selection is considered valid if its total value is within the range : [targetAmount, targetAmount + costOfChange]
            */

            //Compute long term fees
            uint32_t confirmTarget = 1008;
            //TODO: we will have a call to estimateSmartFees (bitcoin core/ explorer side) to estimate long term fees
            //int64_t longTermFees = estimateSmartFees(confirmTarget);
            int64_t longTermFees = DEFAULT_FALLBACK_FEE;

            //Compute cost of change
            auto fixedSize = BitcoinLikeTransactionApi::estimateSize(0,
                                                                     0,
                                                                     currency,
                                                                     buddy->keychain->getKeychainEngine());
            //Size of only 1 output (without fixed size)
            const int64_t oneOutputSize = BitcoinLikeTransactionApi::estimateSize(0,
                                                                         1,
                                                                         currency,
                                                                         buddy->keychain->getKeychainEngine()).Max - fixedSize.Max;
            //Size 1 signed UTXO (signed input)
            const int64_t signedUTXOSize = BitcoinLikeTransactionApi::estimateSize(1,
                                                                          0,
                                                                          currency,
                                                                          buddy->keychain->getKeychainEngine()).Max - fixedSize.Max;

            //Size of unsigned change
            const int64_t changeSize = oneOutputSize;
            //Size of signed change
            const int64_t signedChangeSize = signedUTXOSize;

            const int64_t effectiveFees = buddy->request.feePerByte->toInt64();
            // Here signedChangeSize should be multiplied by discard fees
            // but since we don't have access to estimateSmartFees, we assume
            // that discard fees are equal to effectiveFees
            const int64_t costOfChange = effectiveFees * (signedChangeSize + changeSize);
            buddy->logger->debug("Cost of change {}, signedChangeSize {}, changeSize {}", costOfChange, signedChangeSize, changeSize);

            //Calculate effective value of outputs
            int64_t currentAvailableValue = 0;
            using EffectiveUTXOList = std::vector<EffectiveUTXO>;
            EffectiveUTXOList listEffectiveUTXOs;
            //Get size of utxos as a signed input in a transaction
            for (auto& utxo : utxos) {
                int64_t outEffectiveValue = utxo->getValue()->toLong() - effectiveFees * signedUTXOSize;
                if (outEffectiveValue > 0) {
                    int64_t outEffectiveFees = effectiveFees * signedUTXOSize;
                    int64_t outLongTermFees = longTermFees * signedUTXOSize;
                    listEffectiveUTXOs.emplace_back(EffectiveUTXO{utxo, outEffectiveValue, outEffectiveFees, outLongTermFees});
                    currentAvailableValue += outEffectiveValue;
                }
            }

            //Get no inputs fees
            // At beginning, there are no outputs in tx, so noInputFees are fixed fees
            int64_t notInputFees = effectiveFees * (fixedSize.Max + (int64_t)(oneOutputSize * buddy->request.outputs.size()));//at least fixed size and outputs(version...)

            //Start coin selection algorithm (according to SelectCoinBnb from Bitcoin Core)
            int64_t currentValue = 0;
            std::vector<bool> currentSelection;
            currentSelection.reserve(utxos.size());

            //Actual amount we are targetting
            int64_t actualTarget = notInputFees + buddy->outputAmount.toInt64();

            //Insufficient funds
            if (currentAvailableValue < actualTarget) {
                throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "Cannot gather enough funds.");
            }

            auto descendingEffectiveValue = [] (const EffectiveUTXO &lhs, const EffectiveUTXO &rhs) -> bool {
                return  lhs.effectiveValue > rhs.effectiveValue ;
            };

            //Sort utxos by effectiveValue
            std::sort(listEffectiveUTXOs.begin(), listEffectiveUTXOs.end(), descendingEffectiveValue);

            int64_t currentWaste = 0;
            int64_t bestWaste = MAX_MONEY;
            std::vector<bool> bestSelection;
            buddy->logger->debug("Start filterWithLowestFees, target range is {} to {}, available funds {}", actualTarget, actualTarget + costOfChange, currentAvailableValue);
            //Deep first search loop to choose UTXOs
            for (size_t i = 0; i < TOTAL_TRIES; i++) {

                //Condition for starting a backtrack
                bool backtrack = false;
                if(currentValue + currentAvailableValue < actualTarget || //Cannot reach target with the amount remaining in currentAvailableValue
                   currentValue > actualTarget + costOfChange || // Selected value is out of range, go back and try other branch
                   (currentWaste > bestWaste && listEffectiveUTXOs.at(0).effectiveFees - listEffectiveUTXOs.at(0).longTermFees > 0) ) { //avoid selecting utxos producing more waste
                    backtrack = true;
                } else if (currentValue >= actualTarget) { //Selected valued is within range
                    currentWaste += (currentValue - actualTarget);
                    if (currentWaste <= bestWaste) {
                        bestSelection = currentSelection;
                        bestSelection.resize(listEffectiveUTXOs.size());
                        bestWaste = currentWaste;
                    }
                    // remove the excess value as we will be selecting different coins now
                    currentWaste -= (currentValue - actualTarget);
                    backtrack = true;
                }

                //Move backwards
                if (backtrack) {
                    // Walk backwards to find the last included UTXO that still needs to have its omission branch traversed.
                    while (!currentSelection.empty() && !currentSelection.back()) {
                        currentSelection.pop_back();
                        currentAvailableValue += listEffectiveUTXOs.at(currentSelection.size()).effectiveValue;
                    }

                    //Case we walked back to the first utxo and all solutions searched.
                    if (currentSelection.empty()) {
                        buddy->logger->debug("Current selection is empty, break !");
                        break;
                    }

                    //Output was included on previous iterations, try excluding now
                    currentSelection.back() = false;
                    auto& effectiveUTXO = listEffectiveUTXOs.at(currentSelection.size() - 1);
                    currentValue -= effectiveUTXO.effectiveValue;
                    currentWaste -= (effectiveUTXO.effectiveFees - effectiveUTXO.longTermFees);
                } else { //Moving forwards, continuing down this branch
                    auto& effectiveUTXO = listEffectiveUTXOs.at(currentSelection.size());

                    //Remove this utxo from currentAvailableValue
                    currentAvailableValue -= effectiveUTXO.effectiveValue;

                    // Avoid searching a branch if the previous UTXO has the same value and same waste and was excluded. Since the ratio of fee to
                    // long term fee is the same, we only need to check if one of those values match in order to know that the waste is the same.
                    if (!currentSelection.empty() && !currentSelection.back() &&
                        effectiveUTXO.effectiveValue == listEffectiveUTXOs.at(currentSelection.size() - 1).effectiveValue &&
                        effectiveUTXO.effectiveFees == listEffectiveUTXOs.at(currentSelection.size() - 1).effectiveFees) {
                        currentSelection.push_back(false);
                    } else {
                        //Inclusion branch first
                        currentSelection.push_back(true);
                        currentValue += effectiveUTXO.effectiveValue;
                        currentWaste += (effectiveUTXO.effectiveFees - effectiveUTXO.longTermFees);
                    }
                }
            }

            //If no selection found fallback on filterWithDeepFirst
            if (bestSelection.empty()) {
                buddy->logger->debug("No best selection found, fallback on filterWithKnapsackSolver coin selection");
                return filterWithKnapsackSolver(buddy, utxos, aggregatedAmount, currency);
            }

            //Prepare result
            buddy->logger->debug("Found best selection of size: {}", bestSelection.size());
            UTXODescriptorList out;
            BigInt bestValue = BigInt::ZERO;
            for (size_t i = 0; i < bestSelection.size(); i++) {
                if (bestSelection.at(i)) {
                    buddy->logger->debug("Choose utxo with value: {}", listEffectiveUTXOs.at(i).output->getValue()->toLong());
                    bestValue = bestValue + BigInt(listEffectiveUTXOs.at(i).effectiveValue);
                    UTXODescriptor utxoDescriptor{listEffectiveUTXOs.at(i).output->getTransactionHash(), listEffectiveUTXOs.at(i).output->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())};
                    out.emplace_back(std::move(utxoDescriptor));
                }
            }
            return out;
        }

        static void approximateBestSubset(const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &vUTXOs, const int64_t totalLower, const BigInt &targetValue,
                                          std::vector<bool>& bestValues, int64_t &bestValue, int64_t inputFees, int iterations = 1000) {
            std::vector<bool> includedUTXOs;

            bestValues.assign(vUTXOs.size(), true);
            bestValue = totalLower;

            auto insecureRand = [] () -> bool {
                auto seed = std::chrono::system_clock::now().time_since_epoch().count();
                return std::default_random_engine(seed)() % 2 == 0;
            };

            for (int nRep = 0; nRep < iterations && bestValue != targetValue.toInt64(); nRep++)
            {
                includedUTXOs.assign(vUTXOs.size(), false);
                int64_t total = 0;
                bool fReachedTarget = false;
                for (int nPass = 0; nPass < 2 && !fReachedTarget; nPass++)
                {
                    for (unsigned int i = 0; i < vUTXOs.size(); i++)
                    {
                        //The solver here uses a randomized algorithm,
                        //the randomness serves no real security purpose but is just
                        //needed to prevent degenerate behavior and it is important
                        //that the rng is fast. We do not use a constant random sequence,
                        //because there may be some privacy improvement by making
                        //the selection random.
                        if (nPass == 0 ? insecureRand() : !includedUTXOs[i])
                        {
                            total += vUTXOs[i]->getValue()->toLong() - inputFees;
                            includedUTXOs[i] = true;
                            if (total >= targetValue.toInt64())
                            {
                                fReachedTarget = true;
                                if (total < bestValue)
                                {
                                    bestValue = total;
                                    bestValues = includedUTXOs;
                                }
                                total -= vUTXOs[i]->getValue()->toLong() - inputFees;
                                includedUTXOs[i] = false;
                            }
                        }
                    }
                }
            }

        }

        BitcoinLikeUtxoPicker::UTXODescriptorList BitcoinLikeStrategyUtxoPicker::filterWithKnapsackSolver(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                                                                        const std::vector<std::shared_ptr<api::BitcoinLikeOutput>> &utxos,
                                                                                                        const BigInt &aggregatedAmount,
                                                                                                        const api::Currency& currency
        ) {
            //Tx fixed size
            auto fixedSize = BitcoinLikeTransactionApi::estimateSize(0,
                                                                     0,
                                                                     currency,
                                                                     buddy->keychain->getKeychainEngine());
            //Size of one output (without fixed size)
            const int64_t oneOutputSize = BitcoinLikeTransactionApi::estimateSize(0,
                                                                         1,
                                                                         currency,
                                                                         buddy->keychain->getKeychainEngine()).Max - fixedSize.Max;
            //Size of UTXO as signed input in tx
            const int64_t signedUTXOSize = BitcoinLikeTransactionApi::estimateSize(1,
                                                                          0,
                                                                          currency,
                                                                          buddy->keychain->getKeychainEngine()).Max - fixedSize.Max;

            const int64_t signedUTXOCost = signedUTXOSize * buddy->request.feePerByte->toInt64();

            //Amount + fixed size fees + outputs fees
            const int64_t amountWithFixedFees = buddy->request.feePerByte->toInt64() * (fixedSize.Max + (buddy->request.outputs.size() * oneOutputSize)) + buddy->outputAmount.toInt64();

            // Minimum amount from which we are willing to create a change for it
            // We take the dust as a reference plus the cost of spending this change
            const int64_t minimumChange = currency.bitcoinLikeNetworkParameters->DustAmount + signedUTXOCost;

            buddy->logger->debug("Start filterWithKnapsackSolver, target range is {} to {}", amountWithFixedFees, amountWithFixedFees + minimumChange);
            //List of values less than target
            int64_t totalLower = 0;
            Option<std::shared_ptr<api::BitcoinLikeOutput>> coinLowestLarger;
            std::vector<std::shared_ptr<api::BitcoinLikeOutput>> vUTXOs;
            UTXODescriptorList out;

            //Random shuffle utxos
            std::vector<size_t> indexes;
            for (size_t index = 0; index < utxos.size(); index++) {
                indexes.push_back(index);
            }
            auto seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(indexes.begin(), indexes.end(),std::default_random_engine(seed));
            //Add fees for a signed input to amount
            for (auto index : indexes) {
                auto& utxo = utxos[index];
                               
                //If utxo has exact amount return it
                if (utxo->getValue()->toLong() - signedUTXOCost == amountWithFixedFees) {
                    buddy->logger->debug("Found UTXO with right amount: {}",utxo->getValue()->toLong());
                    UTXODescriptor utxoDescriptor{utxo->getTransactionHash(), utxo->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())};
                    out.emplace_back(utxoDescriptor);
                    return out;
                } else if (utxo->getValue()->toLong() - signedUTXOCost < amountWithFixedFees + minimumChange) { //If utxo in range keep it
                    vUTXOs.push_back(utxo);
                    totalLower += utxo->getValue()->toLong() - signedUTXOCost;
                } else if (!coinLowestLarger.nonEmpty() || utxo->getValue()->toLong() < coinLowestLarger.getValue()->getValue()->toLong()) { //Keep track of lowest utxo out of range
                    buddy->logger->debug("Set lowest out of range UTXO : {} ",utxo->getValue()->toLong());
                    coinLowestLarger = utxo;
                }
            }

            //If exact amount, return vUTXOs
            if (totalLower == amountWithFixedFees) {
                buddy->logger->debug("Total of lower utxos and amount equal");
                for (auto& selectedUTXO : vUTXOs) {
                    UTXODescriptor utxoDescriptor{selectedUTXO->getTransactionHash(), selectedUTXO->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())};
                    out.emplace_back(utxoDescriptor);
                }
                return out;
            } else if (totalLower < amountWithFixedFees) {
                buddy->logger->debug("Total of lower utxos lower than amount equal");
                //If total lower then and no coinLowestLarger then use filterWithDeepFirst
                if (!coinLowestLarger.nonEmpty()) {
                    buddy->logger->debug("No best selection found, fallback on filterWithDeepFirst coin selection");
                    return filterWithDeepFirst(buddy, utxos, buddy->outputAmount, currency);
                }
            }

            //Sort vUTXOs descending
            auto descending = [] (const std::shared_ptr<api::BitcoinLikeOutput> &lhs,
                                  const std::shared_ptr<api::BitcoinLikeOutput> &rhs) -> bool {
                return  lhs->getValue()->toLong() > rhs->getValue()->toLong() ;
            };
            std::sort(vUTXOs.begin(), vUTXOs.end(), descending);

            //Approximate best tries
            std::vector<bool> bestValues;
            int64_t bestValue = 0;
            buddy->logger->debug("Approximate Best Subset 1st try");
            // Here we target the value amountWithFixedFees which is the amount of tx + fixed fees (fees of transaction without signed UTXOs)
            approximateBestSubset(vUTXOs, totalLower, BigInt(static_cast<int64_t>(amountWithFixedFees)), bestValues, bestValue, signedUTXOCost);
            if (bestValue != amountWithFixedFees && totalLower >= amountWithFixedFees + minimumChange) {
                buddy->logger->debug("First approximation, bestValue {} with {} bestValues", bestValue, bestValues.size());
                buddy->logger->debug("Approximate Best Subset 2nd try");
                approximateBestSubset(vUTXOs, totalLower, BigInt((int64_t)amountWithFixedFees + minimumChange), bestValues, bestValue, signedUTXOCost);
                buddy->logger->debug("Second approximation, bestValue {} with {} bestValues", bestValue, bestValues.size());
            }

            // We compute the totalBest here because we need picked UTXOs in subset
            // to be able to update amountWithFees (because after signing all UTXOs we might need to pick additional UTXOs to cover
            // fees generated by adding this UTXOs and their signature)
            int64_t totalBest = 0;
            UTXODescriptorList tmpOut;
            for (unsigned int i = 0; i < vUTXOs.size(); i++) {
                if (bestValues[i])
                {
                    UTXODescriptor utxoDescriptor{vUTXOs[i]->getTransactionHash(), vUTXOs[i]->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())};
                    tmpOut.emplace_back(utxoDescriptor);
                    totalBest += vUTXOs[i]->getValue()->toLong();
                }
            }

            buddy->logger->debug("Total Best = {}", totalBest);

            //If bestValue is different from amountWithFees and coinLowestLarger is lower than bestVaule, then choose coinLowestLarger
            if (coinLowestLarger.nonEmpty() &&
                ((bestValue != amountWithFixedFees && bestValue < amountWithFixedFees + minimumChange) ||
                 coinLowestLarger.getValue()->getValue()->toLong() - signedUTXOCost <= bestValue))
            {
                buddy->logger->debug("Add coinLowestLarger to coin selection");
                UTXODescriptor utxoDescriptor{coinLowestLarger.getValue()->getTransactionHash(), coinLowestLarger.getValue()->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())};
                out.emplace_back(utxoDescriptor);

                buddy->changeAmount =  BigInt(coinLowestLarger.getValue()->getValue()->toLong() - signedUTXOCost - (int64_t)(amountWithFixedFees + oneOutputSize * buddy->request.feePerByte->toInt64()));
            }
            else { //Pick bestValues
                buddy->logger->debug("Push all vUTXOs");
                out = tmpOut;
                // Set amount of change
                // Change amount = amountWithFees + fees for 1 additional output (change)
                buddy->changeAmount =  BigInt(bestValue - (int64_t)(amountWithFixedFees + oneOutputSize * buddy->request.feePerByte->toInt64()));
            }
            if (buddy->changeAmount.toInt64() < minimumChange) {
                buddy->changeAmount = BigInt(0);
            }
            return out;
        }

        BitcoinLikeUtxoPicker::UTXODescriptorList BitcoinLikeStrategyUtxoPicker::filterWithMergeOutputs(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy>& buddy,
                                                                                                        const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& utxos,
                                                                                                        const BigInt& aggregatedAmount,
                                                                                                        const api::Currency& currency) {
            buddy->logger->debug("Start filterWithMergeOutputs");
            auto sortedUTXOs = utxos;
            auto ascending = [] (const std::shared_ptr<api::BitcoinLikeOutput> &lhs,
                                 const std::shared_ptr<api::BitcoinLikeOutput> &rhs) -> bool {
                return  lhs->getValue()->toLong() < rhs->getValue()->toLong() ;
            };

            std::sort(sortedUTXOs.begin(), sortedUTXOs.end(), ascending);

            UTXODescriptorList out;
            BigInt amount = aggregatedAmount;
            int pickedInputs = 0;
            for (auto& utxo : sortedUTXOs) {

                pickedInputs += 1;
                amount = amount + BigInt(utxo->getValue()->toLong());
                UTXODescriptor utxoDescriptor{utxo->getTransactionHash(), utxo->getOutputIndex(), std::get<1>(buddy->request.utxoPicker.getValue())};
                out.emplace_back(std::move(utxoDescriptor));
                bool computeOutputAmount = pickedInputs == sortedUTXOs.size();
                if (hasEnough(buddy, amount, pickedInputs, currency, computeOutputAmount)) {
                    break;
                }

                if (pickedInputs >= sortedUTXOs.size() && !buddy->request.wipe) {
                    throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "Cannot gather enough funds.");
                }
            }

            return out;
        }

    }
}

