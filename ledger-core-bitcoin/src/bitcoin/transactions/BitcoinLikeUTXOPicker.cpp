/*
 *
 * BitcoinLikeUTXOPicker.cpp
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

#include <core/async/Promise.hpp>
#include <core/utils/ImmediateExecutionContext.hpp>

#include <bitcoin/api/BitcoinLikeScript.hpp>
#include <bitcoin/api/BitcoinLikeScriptChunk.hpp>
#include <bitcoin/scripts/BitcoinLikeInternalScript.hpp>
#include <bitcoin/transactions/BitcoinLikeTransaction.hpp>
#include <bitcoin/transactions/BitcoinLikeUTXOPicker.hpp>

namespace ledger {
    namespace core {


        BitcoinLikeUTXOPicker::BitcoinLikeUTXOPicker(const std::shared_ptr<api::ExecutionContext> &context,
                                                     const api::Currency &currency) : DedicatedContext(context),
                                                                                      _currency(currency)
        {}

        BitcoinLikeTransactionBuildFunction
        BitcoinLikeUTXOPicker::getBuildFunction(const BitcoinLikeGetUTXOFunction &getUTXO,
                                                const BitcoinLikeGetTxFunction& getTransaction,
                                                const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer,
                                                const std::shared_ptr<BitcoinLikeKeychain> &keychain,
                                                const uint64_t currentBlockHeight,
                                                const std::shared_ptr<spdlog::logger>& logger,
                                                bool partial)
        {
            auto self = shared_from_this();
            logger->info("Get build function");
            return [=] (const BitcoinLikeTransactionBuildRequest& r) -> Future<std::shared_ptr<api::BitcoinLikeTransaction>> {
                return self->async<std::shared_ptr<Buddy>>([=] () {
                    logger->info("Constructing BitcoinLikeTransactionBuildFunction with blockHeight: {}", currentBlockHeight);
                    auto tx = std::make_shared<BitcoinLikeTransaction>(self->_currency, keychain->getKeychainEngine(), currentBlockHeight);
                    auto filteredGetUTXO = createFilteredUTXOFunction(r, getUTXO);
                    return std::make_shared<Buddy>(r, filteredGetUTXO, getTransaction, explorer, keychain, logger, tx, partial);
                }).flatMap<std::shared_ptr<api::BitcoinLikeTransaction>>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<Buddy>& buddy) -> Future<std::shared_ptr<api::BitcoinLikeTransaction>> {
                    buddy->logger->info("Buddy created");
                    return self->fillInputs(buddy).flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const Unit&) -> Future<Unit> {
                        return self->fillOutputs(buddy);
                    }).flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const Unit&) -> Future<Unit> {
                        return self->fillTransactionInfo(buddy);
                    }).mapPtr<api::BitcoinLikeTransaction>(ImmediateExecutionContext::INSTANCE, [=] (const Unit&) -> std::shared_ptr<api::BitcoinLikeTransaction> {
                        buddy->logger->info("Empty transaction built");
                        return buddy->transaction;
                    });;
                });
            };;
        }

        const api::Currency &BitcoinLikeUTXOPicker::getCurrency() const {
            return _currency;
        }

        Future<Unit> BitcoinLikeUTXOPicker::fillOutputs(const std::shared_ptr<Buddy>& buddy) {
            // Fill reception outputs
            auto params = networks::getBitcoinLikeNetworkParameters(getCurrency().name);
            auto outputIndex = 0;
            for (auto &output : buddy->request.outputs) {
                auto amount = std::get<0>(output);
                //Set right amount if we are in wipe mode
                if(buddy->request.wipe) {
                    amount = std::make_shared<ledger::core::BigInt>(buddy->outputAmount);
                }
                auto script = std::dynamic_pointer_cast<BitcoinLikeInternalScript>(std::get<1>(output))->getScript();
                auto address = script.parseAddress(getCurrency()).map<std::string>([] (const BitcoinLikeAddress& addr) {
                    return addr.getStringAddress();
                });
                BitcoinLikeBlockchainExplorerOutput out;
                out.index = static_cast<uint64_t>(outputIndex);
                out.value = *amount;
                out.address = address;
                out.script = hex::toString(script.serialize());
                std::shared_ptr<api::DerivationPath> derivationPath = nullptr;
                if (address.nonEmpty()) {
                    auto path = buddy->keychain->getAddressDerivationPath(out.address.getValue());
                    if (path.nonEmpty()) {
                        derivationPath = std::make_shared<DerivationPath>(DerivationPath(path.getValue()));
                    }
                } else {
                    auto addressFromScript = script.parseAddress(getCurrency());
                    if (addressFromScript.nonEmpty())
                        out.address = addressFromScript.getValue().toString();
                }
                outputIndex += 1;
                buddy->transaction->addOutput(std::make_shared<BitcoinLikeOutput>(out, getCurrency(), derivationPath));
            }

            // Fill change outputs

            if (buddy->changeAmount > BigInt(networks::getBitcoinLikeNetworkParameters(_currency.name).DustAmount)) {
                // TODO implement multi change
                // TODO implement use specific change address
                auto changeAddress = buddy->keychain->getFreshAddress(BitcoinLikeKeychain::CHANGE)->toString();

                auto amount = buddy->changeAmount;
                auto script = BitcoinLikeScript::fromAddress(changeAddress, _currency);
                BitcoinLikeBlockchainExplorerOutput out;
                out.index = static_cast<uint64_t>(buddy->transaction->getOutputs().size());
                out.value = amount;
                out.address = Option<std::string>(changeAddress).toOptional();
                out.script = hex::toString(script.serialize());
                std::shared_ptr<DerivationPath> derivationPath = nullptr;
                auto path = buddy->keychain->getAddressDerivationPath(changeAddress);
                if (path.nonEmpty()) {
                    derivationPath = std::make_shared<DerivationPath>(DerivationPath(path.getValue()));
                }
                buddy->transaction->addOutput(std::make_shared<BitcoinLikeOutput>(out, getCurrency(), derivationPath));
            }

            return Future<Unit>::successful(unit);
        }

        Future<Unit> BitcoinLikeUTXOPicker::fillTransactionInfo(const std::shared_ptr<Buddy>& buddy) {
            if (buddy->isPartial) {
                return Future<Unit>::successful(unit);
            }
            //Set timestamp
            buddy->explorer->getTimestamp().onComplete(ImmediateExecutionContext::INSTANCE, [=] (const Try<int64_t> &timestamp){
                if (timestamp.isSuccess()) {
                    buddy->transaction->setTimestamp(timestamp.getValue());
                }
            });

            return buddy->explorer->getCurrentBlock().map<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::Block>& block) -> Unit{
                buddy->transaction->setLockTime(static_cast<uint32_t>(block->height));
                return unit;
            });
        }

        Future<Unit> BitcoinLikeUTXOPicker::fillInputs(const std::shared_ptr<Buddy>& buddy) {
            buddy->logger->info("Filling inputs");
            auto self = shared_from_this();
            auto pickUTXO = [=] () -> Future<UTXODescriptorList> {
                if (buddy->request.UTXOPicker.nonEmpty()) {
                   return self->filterInputs(buddy);
                }
                return Future<UTXODescriptorList>::successful(std::move(UTXODescriptorList()));
            };

            auto inputs = std::make_shared<UTXODescriptorList>();
            static std::function<Future<Unit> (int, const std::shared_ptr<UTXODescriptorList>&, const std::shared_ptr<Buddy>&, const std::shared_ptr<BitcoinLikeUTXOPicker>&)> performFill
            = [] (int index, const std::shared_ptr<UTXODescriptorList>& inputs, const std::shared_ptr<Buddy>& buddy, const std::shared_ptr<BitcoinLikeUTXOPicker>& self) mutable -> Future<Unit> {
                if (index >= inputs->size())
                    return Future<Unit>::successful(unit);
                return self->fillInput(buddy, (*inputs)[index]).flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const Unit&) {
                    return performFill(index + 1, inputs, buddy, self);
                });
            };
            inputs->insert(inputs->end(), buddy->request.inputs.begin(), buddy->request.inputs.end());
            return performFill(0, inputs, buddy, self).flatMap<UTXODescriptorList>(ImmediateExecutionContext::INSTANCE, [=] (const Unit&) mutable -> Future<UTXODescriptorList> {
                return pickUTXO();
            }).flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const UTXODescriptorList& UTXO) mutable -> Future<Unit> {
                auto offset = static_cast<int>(inputs->size());
                inputs->insert(inputs->end(), UTXO.begin(), UTXO.end());
                return performFill(offset, inputs, buddy, self);
            });
        }

        Future<Unit> BitcoinLikeUTXOPicker::fillInput(const std::shared_ptr<BitcoinLikeUTXOPicker::Buddy> &buddy,
                                                      const BitcoinLikeUTXOPicker::UTXODescriptor &desc) {
            const std::string& hash = std::get<0>(desc);
            auto txGetter = [=] (const std::string &hash) -> FuturePtr<BitcoinLikeBlockchainExplorerTransaction> {
                if (buddy->isPartial) {
                    return buddy->getTransaction(hash);
                }
                return buddy->explorer->getTransactionByHash(hash);
            };
            return txGetter(hash).map<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorerTransaction>& tx) {
                buddy->logger->debug("Get output {} on {}", std::get<1>(desc), tx->outputs.size());
                auto output = tx->outputs[std::get<1>(desc)];
                std::vector<std::vector<uint8_t>> pub_keys;
                std::vector<std::shared_ptr<api::DerivationPath>> paths;

                // Get derivations and public keys
                if (output.address.nonEmpty()) {
                    auto derivationPath = buddy->keychain->getAddressDerivationPath(output.address.getValue());
                    if (derivationPath.nonEmpty()) {
                        paths.push_back(std::make_shared<DerivationPath>(DerivationPath(derivationPath.getValue())));
                        pub_keys.push_back(buddy->keychain->getPublicKey(output.address.getValue()).getValue());
                    }
                }
                auto input = std::shared_ptr<BitcoinLikeWritableInput>(
                        new BitcoinLikeWritableInput(
                                buddy->explorer, getContext(), std::get<2>(desc), pub_keys, paths,
                                output.address.getValueOr(""),
                                std::make_shared<Amount>(buddy->keychain->getCurrency(), 0, output.value),
                                hash, std::get<1>(desc), {}, std::make_shared<BitcoinLikeOutput>(output, _currency)
                        )
                );
                buddy->transaction->addInput(input);
                return unit;
            });
        }

        BitcoinLikeGetUTXOFunction
        BitcoinLikeUTXOPicker::createFilteredUTXOFunction(const BitcoinLikeTransactionBuildRequest &request,
                                                          const BitcoinLikeGetUTXOFunction &getUTXO) {
            return [=] () -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                return getUTXO().map<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>(getContext(), [=] (const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& UTXO) {
                    std::vector<std::shared_ptr<api::BitcoinLikeOutput>> filtered;
                    auto isExcluded = [&] (const std::shared_ptr<api::BitcoinLikeOutput>& output) -> bool {
                        for (auto& o : request.excludedUTXO) {
                            auto hash = std::get<0>(o);
                            auto index = std::get<1>(o);
                            if (output->getTransactionHash() == hash && output->getOutputIndex() == index)
                                return true;
                        }
                        return false;
                    };
                    for (auto& output : UTXO) {
                        if (!isExcluded(output)) {
                            filtered.push_back(output);
                        }
                    }
                    return filtered;
                });
            };
        }
    }
}