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
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>

#include <random>
#include <numeric>

namespace ledger {
    namespace core {

        BitcoinLikeStrategyUtxoPicker::BitcoinLikeStrategyUtxoPicker(const std::shared_ptr<api::ExecutionContext> &context,
                                                                     const api::Currency &currency) : BitcoinLikeUtxoPicker(context, currency) {

        }

        Future<std::vector<BitcoinLikeUtxo>>
        BitcoinLikeStrategyUtxoPicker::filterInputs(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy) {
            return computeAggregatedAmount(buddy).flatMap<std::vector<BitcoinLikeUtxo>>(getContext(), [=] (BigInt const &amount) {
                buddy->logger->info("GET UTXO");

                return buddy->getUtxo().map<std::vector<BitcoinLikeUtxo>>(
                    getContext(),
                    [=] (std::vector<BitcoinLikeUtxo> const &utxos) {
                        buddy->logger->info("GOT UTXO");

                        if (utxos.size() == 0)
                            throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "There is no UTXO on this account.");

                        //If wipe mode no matter which strategy we use, let's use filterWithDeepFirst (for the moment)
                        if (buddy->request.wipe) {
                            return filterWithDeepFirst(buddy, utxos, amount, getCurrency());
                        }

                        auto picker = buddy->request.utxoPicker.getValue();
                        const api::BitcoinLikePickingStrategy strategy = std::get<0>(picker);

                        switch (strategy) {
                            case api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST:
                                return filterWithDeepFirst(buddy, utxos, amount, getCurrency());
                            case api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE:
                                return filterWithOptimizeSize(buddy, utxos, amount, getCurrency());
                            case api::BitcoinLikePickingStrategy::MERGE_OUTPUTS:
                                return filterWithMergeOutputs(buddy, utxos, amount, getCurrency());
                        }
                    });
            });
        }

        Future<BigInt> BitcoinLikeStrategyUtxoPicker::computeAggregatedAmount(
            const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy) {
            // NOTE: We can turn that lambda to a free function and pass the context and the buddy as arguments
            auto go = [context = getContext(), buddy] (auto go, auto it, auto v) {
                if (it == buddy->request.inputs.end())
                    return Future<BigInt>::successful(v);
                const auto& i = *it;
                const auto outputIndex = i.outputIndex;
                // NOTE: why always 1 ?
                buddy->logger->info("GET TX 1");
                return buddy->getTransaction(i.transactionHash).template flatMap<BigInt>(context, [=] (auto const& tx) mutable {
                    buddy->logger->info("GOT TX 1");
                    return go(go, ++it, v + tx->outputs[outputIndex].value);
                });
            };
            // NOTE: we can now use buddy->transaction->inputs which is filled during fillInputs by user inputs
            return go(go, buddy->request.inputs.begin(), BigInt());
        }

        std::vector<BitcoinLikeUtxo>
        BitcoinLikeStrategyUtxoPicker::filterWithDeepFirst(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                            const std::vector<BitcoinLikeUtxo> &utxos,
                                                            const BigInt &aggregatedAmount,
                                                            const api::Currency& currency) {

            buddy->logger->debug("Start filterWithDeepFirst");

            return filterWithSort(buddy, utxos, aggregatedAmount, currency, [] (auto &lhs, auto &rhs) {
                constexpr auto maxBlockHeight = std::numeric_limits<uint64_t>::max();
                return lhs.blockHeight.getValueOr(maxBlockHeight) < rhs.blockHeight.getValueOr(maxBlockHeight);
            });
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
                buddy->logger->debug("Estimate for {} inputs with {} outputs", inputCount, buddy->request.outputs.size() + addedOutputCount);
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

        std::vector<BitcoinLikeUtxo>
        BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                              const std::vector<BitcoinLikeUtxo> &utxos,
                                                              const BigInt &aggregatedAmount,
                                                              const api::Currency& currency) {

            // NOTE: why are we using buddy->outputAmount here instead of aggregatedAmount ?
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
            auto const fixedSize = BitcoinLikeTransactionApi::estimateSize(0,
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

            struct EffectiveUtxo {
                const BitcoinLikeUtxo *utxo;
                int64_t effectiveValue;
                int64_t effectiveFees;
                int64_t longTermFees;
            };

            std::vector<EffectiveUtxo> effectiveUtxos;
            //Get size of utxos as a signed input in a transaction
            for (auto& utxo : utxos) {
                int64_t outEffectiveValue = utxo.value.toLong() - effectiveFees * signedUTXOSize;
                if (outEffectiveValue > 0) {
                    int64_t outEffectiveFees = effectiveFees * signedUTXOSize;
                    int64_t outLongTermFees = longTermFees * signedUTXOSize;
                    effectiveUtxos.push_back(EffectiveUtxo{&utxo, outEffectiveValue, outEffectiveFees, outLongTermFees});
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

            auto descendingEffectiveValue = [] (const EffectiveUtxo &lhs, const EffectiveUtxo &rhs) -> bool {
                return  lhs.effectiveValue > rhs.effectiveValue ;
            };

            //Sort utxos by effectiveValue
            std::sort(effectiveUtxos.begin(), effectiveUtxos.end(), descendingEffectiveValue);

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
                   (currentWaste > bestWaste && effectiveUtxos.at(0).effectiveFees - effectiveUtxos.at(0).longTermFees > 0) ) { //avoid selecting utxos producing more waste
                    backtrack = true;
                } else if (currentValue >= actualTarget) { //Selected valued is within range
                    currentWaste += (currentValue - actualTarget);
                    if (currentWaste <= bestWaste) {
                        bestSelection = currentSelection;
                        bestSelection.resize(effectiveUtxos.size());
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
                        currentAvailableValue += effectiveUtxos.at(currentSelection.size()).effectiveValue;
                    }

                    //Case we walked back to the first utxos and all solutions searched.
                    if (currentSelection.empty()) {
                        buddy->logger->debug("Current selection is empty, break !");
                        break;
                    }

                    //Output was included on previous iterations, try excluding now
                    currentSelection.back() = false;
                    auto& eu = effectiveUtxos.at(currentSelection.size() - 1);
                    currentValue -= eu.effectiveValue;
                    currentWaste -= (eu.effectiveFees - eu.longTermFees);
                } else { //Moving forwards, continuing down this branch
                    auto& eu = effectiveUtxos.at(currentSelection.size());

                    //Remove this utxos from currentAvailableValue
                    currentAvailableValue -= eu.effectiveValue;

                    // Avoid searching a branch if the previous UTXO has the same value and same waste and was excluded. Since the ratio of fee to
                    // long term fee is the same, we only need to check if one of those values match in order to know that the waste is the same.
                    if (!currentSelection.empty() && !currentSelection.back() &&
                        eu.effectiveValue == effectiveUtxos.at(currentSelection.size() - 1).effectiveValue &&
                        eu.effectiveFees == effectiveUtxos.at(currentSelection.size() - 1).effectiveFees) {
                        currentSelection.push_back(false);
                    } else {
                        //Inclusion branch first
                        currentSelection.push_back(true);
                        currentValue += eu.effectiveValue;
                        currentWaste += (eu.effectiveFees - eu.longTermFees);
                    }
                }
            }

            // NOTE: we're using filterWithKnapsackSolver instead of filterWithDeepFirst
            //If no selection found fallback on filterWithDeepFirst
            if (bestSelection.empty()) {
                buddy->logger->debug("No best selection found, fallback on filterWithKnapsackSolver coin selection");
                return filterWithKnapsackSolver(buddy, utxos, aggregatedAmount, currency);
            }

            //Prepare result
            buddy->logger->debug("Found best selection of size: {}", bestSelection.size());

            std::vector<BitcoinLikeUtxo> out;
            BigInt bestValue = BigInt::ZERO;
            for (size_t i = 0; i < bestSelection.size(); i++) {
                if (bestSelection.at(i)) {
                    auto const &eu = effectiveUtxos.at(i);

                    buddy->logger->debug("Choose utxos with value: {}", eu.utxo->value.toLong());
                    bestValue = bestValue + BigInt(eu.effectiveValue);
                    out.push_back(std::move(*eu.utxo));
                }
            }
            return out;
        }

        static void approximateBestSubset(const std::vector<BitcoinLikeUtxo> &vUTXOs, const int64_t totalLower, const BigInt &targetValue,
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
                            auto currentAmount = vUTXOs[i].value.toLong() - inputFees;

                            total += currentAmount;
                            includedUTXOs[i] = true;
                            if (total >= targetValue.toInt64())
                            {
                                fReachedTarget = true;
                                if (total < bestValue)
                                {
                                    bestValue = total;
                                    bestValues = includedUTXOs;
                                }
                                total -= currentAmount;
                                includedUTXOs[i] = false;
                            }
                        }
                    }
                }
            }

        }

        std::vector<BitcoinLikeUtxo> BitcoinLikeStrategyUtxoPicker::filterWithKnapsackSolver(
                const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                const std::vector<BitcoinLikeUtxo> &utxos,
                const BigInt &aggregatedAmount,
                const api::Currency& currency) {

            //Tx fixed size
            auto const fixedSize = BitcoinLikeTransactionApi::estimateSize(0,
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
            Option<BitcoinLikeUtxo> coinLowestLarger;
            std::vector<BitcoinLikeUtxo> vUTXOs;
            std::vector<BitcoinLikeUtxo> out;

            //Random shuffle utxos
            std::vector<size_t> indexes(utxos.size());
            std::iota(indexes.begin(), indexes.end(), 0);

            auto const seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(indexes.begin(), indexes.end(), std::default_random_engine(seed));

            //Add fees for a signed input to amount
            for (auto index : indexes) {
                auto& utxo = utxos[index];

                const int64_t currentAmount = utxo.value.toLong();
                const int64_t currentAmountWithDeductedCost = currentAmount - signedUTXOCost;
                if (currentAmountWithDeductedCost == amountWithFixedFees) {
                    buddy->logger->debug("Found UTXO with right amount: {}", currentAmount);
                    out.push_back(utxo);
                    return out;
                } else if (currentAmountWithDeductedCost < amountWithFixedFees + minimumChange) { //If utxo in range keep it
                    vUTXOs.push_back(utxo);
                    totalLower += currentAmountWithDeductedCost;
                } else if (!coinLowestLarger.nonEmpty() || currentAmount < coinLowestLarger.getValue().value.toLong()) { //Keep track of lowest utxos out of range
                    buddy->logger->debug("Set lowest out of range UTXO : {} ", currentAmount);
                    coinLowestLarger = utxo;
                }
            }

            //If exact amount, return vUTXOs
            if (totalLower == amountWithFixedFees) {
                buddy->logger->debug("Total of lower utxos and amount equal");
                for (auto& selectedUtxo : vUTXOs) {
                    out.push_back(selectedUtxo);
                }
                return out;
            } else if (totalLower < amountWithFixedFees) {
                buddy->logger->debug("Total of lower utxos lower than amount equal");
                //If total lower then and no coinLowestLarger then use filterWithDeepFirst
                if (!coinLowestLarger.nonEmpty()) {
                    buddy->logger->debug("No best selection found, fallback on filterWithDeepFirst coin selection");
                    // NOTE: same question here why we use buddy->outputAmount instead of aggregatedAmount
                    return filterWithDeepFirst(buddy, utxos, buddy->outputAmount, currency);
                }
            }

            //Sort vUTXOs descending
            std::sort(vUTXOs.begin(), vUTXOs.end(), [](auto const &lhs, auto const &rhs) {
                return lhs.value.toLong() > rhs.value.toLong();
            });

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
            // to be able to update amountWithFixedFees (because after signing all UTXOs we might need to pick additional UTXOs to cover
            // fees generated by adding this UTXOs and their signature)
            int64_t totalBest = 0;
            std::vector<BitcoinLikeUtxo> tmpOut;
            for (unsigned int i = 0; i < vUTXOs.size(); i++) {
                if (bestValues[i])
                {
                    auto const& utxo = vUTXOs[i];

                    tmpOut.push_back(utxo);
                    totalBest += utxo.value.toLong();
                }
            }

            buddy->logger->debug("Total Best = {}", totalBest);

            //If bestValue is different from amountWithFixedFees and coinLowestLarger is lower than bestVaule, then choose coinLowestLarger
            if (coinLowestLarger.nonEmpty() &&
                ((bestValue != amountWithFixedFees && bestValue < amountWithFixedFees + minimumChange) ||
                 coinLowestLarger.getValue().value.toLong() - signedUTXOCost <= bestValue))
            {
                buddy->logger->debug("Add coinLowestLarger to coin selection");
                out.push_back(coinLowestLarger.getValue());

                buddy->changeAmount =  BigInt(coinLowestLarger.getValue().value.toLong() - signedUTXOCost - (int64_t)(amountWithFixedFees + oneOutputSize * buddy->request.feePerByte->toInt64()));
            }
            else { //Pick bestValues
                buddy->logger->debug("Push all vUTXOs");
                out = tmpOut;
                // Set amount of change
                // Change amount = amountWithFixedFees + fees for 1 additional output (change)
                buddy->changeAmount =  BigInt(bestValue - (int64_t)(amountWithFixedFees + oneOutputSize * buddy->request.feePerByte->toInt64()));
            }

            if (buddy->changeAmount.toInt64() < minimumChange) {
                buddy->changeAmount = BigInt(0);
            }

            return out;
        }

        std::vector<BitcoinLikeUtxo> BitcoinLikeStrategyUtxoPicker::filterWithMergeOutputs(
                const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                const std::vector<BitcoinLikeUtxo> &utxos,
                const BigInt &aggregatedAmount,
                const api::Currency& currency) {

            buddy->logger->debug("Start filterWithMergeOutputs");

            return filterWithSort(buddy, utxos, aggregatedAmount, currency, [](auto &lhs, auto &rhs) {
                return lhs.value.toLong() < rhs.value.toLong();
            });
        }

        std::vector<BitcoinLikeUtxo> BitcoinLikeStrategyUtxoPicker::filterWithSort(
                const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                std::vector<BitcoinLikeUtxo> utxos,
                BigInt amount,
                const api::Currency& currency,
                std::function<bool(BitcoinLikeUtxo&, BitcoinLikeUtxo&)> const& functor)
        {
            auto pickedUtxos = std::vector<BitcoinLikeUtxo>{};
            auto pickedInputs = 0;

            pickedUtxos.reserve(utxos.size());
            std::sort(utxos.begin(), utxos.end(), functor);

            bool enough = false;
            for (auto const &u : utxos) {
                amount = amount + *u.value.value();
                pickedInputs += 1;
                pickedUtxos.push_back(u);

                buddy->logger->debug("Collected: {} Needed: {}", amount.toString(), buddy->outputAmount.toString());

                auto const computeOutputAmount = pickedInputs == utxos.size();
                if (hasEnough(buddy, amount, pickedInputs, currency, computeOutputAmount)) {
                    enough = true;
                    break;
                }
            }

            if (!enough && !buddy->request.wipe) {
                throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "Cannot gather enough funds.");
            }

            buddy->logger->debug("Require {} inputs to complete the transaction with {} for {}", pickedInputs, amount.toString(), buddy->outputAmount.toString());

            pickedUtxos.shrink_to_fit();

            return pickedUtxos;
        }
    }
}

