/*
 *
 * BitcoinLikeUtxoPicker.cpp
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

#include "BitcoinLikeUtxoPicker.h"
#include <async/Promise.hpp>
#include <api/BitcoinLikeScript.hpp>
#include <api/BitcoinLikeScriptChunk.hpp>
#include <wallet/bitcoin/api_impl/BitcoinLikeScriptApi.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <utils/ImmediateExecutionContext.hpp>

namespace ledger {
    namespace core {


        BitcoinLikeUtxoPicker::BitcoinLikeUtxoPicker(const std::shared_ptr<api::ExecutionContext> &context,
                                                     const api::Currency &currency) : DedicatedContext(context),
                                                                                      _currency(currency)
        {}

        BitcoinLikeTransactionBuildFunction
        BitcoinLikeUtxoPicker::getBuildFunction(const BitcoinLikeGetUtxoFunction &getUtxo,
                                                const BitcoinLikeGetTxFunction& getTransaction,
                                                const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer,
                                                const std::shared_ptr<BitcoinLikeKeychain> &keychain,
                                                const std::shared_ptr<spdlog::logger>& logger
        ) {
            auto self = shared_from_this();
            logger->info("Get build function");
            return [=] (const BitcoinLikeTransactionBuildRequest& r) -> Future<std::shared_ptr<api::BitcoinLikeTransaction>> {
                return self->async<std::shared_ptr<Buddy>>([=] () {
                    auto tx = std::make_shared<BitcoinLikeTransactionApi>(self->_currency);
                    auto filteredGetUtxo = createFilteredUtxoFunction(r, getUtxo);
                    return std::make_shared<Buddy>(r, filteredGetUtxo, getTransaction, explorer, keychain, logger, tx);
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
            };
        }

        const api::Currency &BitcoinLikeUtxoPicker::getCurrency() const {
            return _currency;
        }

        Future<Unit> BitcoinLikeUtxoPicker::fillOutputs(const std::shared_ptr<Buddy>& buddy) {
            // Fill reception outputs
            auto params = getCurrency().bitcoinLikeNetworkParameters.value();
            auto outputIndex = 0;
            for (auto &output : buddy->request.outputs) {
                auto amount = std::get<0>(output);
                auto script = std::dynamic_pointer_cast<BitcoinLikeScriptApi>(std::get<1>(output))->getScript();
                auto address = script.parseAddress(params).map<std::string>([] (const BitcoinLikeAddress& addr) {
                    return addr.toBase58();
                });
                BitcoinLikeBlockchainExplorer::Output out;
                out.index = static_cast<uint64_t>(outputIndex);
                out.value = *amount;
                out.address = address;
                out.script = hex::toString(script.serialize());
                std::shared_ptr<api::DerivationPath> derivationPath = nullptr;
                if (address.nonEmpty()) {
                    auto path = buddy->keychain->getAddressDerivationPath(out.address.getValue());
                    if (path.nonEmpty()) {
                        derivationPath = std::make_shared<DerivationPathApi>(DerivationPath(path.getValue()));
                    }
                } else {
                    auto addressFromScript = script.parseAddress(params);
                    if (addressFromScript.nonEmpty())
                        out.address = addressFromScript.getValue().toBase58();
                }
                outputIndex += 1;
                buddy->transaction->addOutput(std::make_shared<BitcoinLikeOutputApi>(out, getCurrency(), derivationPath));
            }

            // Fill change outputs

            if (buddy->changeAmount > BigInt(_currency.bitcoinLikeNetworkParameters.value().DustAmount)) {
                // TODO implement multi change
                // TODO implement use specific change address
                auto changeAddress = buddy->keychain->getFreshAddress(BitcoinLikeKeychain::CHANGE);

                auto amount = buddy->changeAmount;
                auto script = BitcoinLikeScript::fromAddress(changeAddress, _currency.bitcoinLikeNetworkParameters.value());
                BitcoinLikeBlockchainExplorer::Output out;
                out.index = static_cast<uint64_t>(buddy->transaction->getOutputs().size());
                out.value = amount;
                out.address = Option<std::string>(changeAddress).toOptional();
                out.script = hex::toString(script.serialize());
                std::shared_ptr<api::DerivationPath> derivationPath = nullptr;
                auto path = buddy->keychain->getAddressDerivationPath(changeAddress);
                if (path.nonEmpty()) {
                    derivationPath = std::make_shared<DerivationPathApi>(DerivationPath(path.getValue()));
                }
                buddy->transaction->addOutput(std::make_shared<BitcoinLikeOutputApi>(out, getCurrency(), derivationPath));
            }

            return Future<Unit>::successful(unit);
        }

        Future<Unit> BitcoinLikeUtxoPicker::fillTransactionInfo(const std::shared_ptr<Buddy>& buddy) {
            return buddy->explorer->getCurrentBlock().map<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::Block>& block) -> Unit{
                buddy->transaction->setLockTime(static_cast<uint32_t>(block->height));
                return unit;
            });
        }

        Future<Unit> BitcoinLikeUtxoPicker::fillInputs(const std::shared_ptr<Buddy>& buddy) {
            buddy->logger->info("Filling inputs");
            auto self = shared_from_this();
            auto pickUtxo = [=] () -> Future<UTXODescriptorList> {
                if (buddy->request.utxoPicker.nonEmpty()) {
                   return self->filterInputs(buddy);
                }
                return Future<UTXODescriptorList>::successful(std::move(UTXODescriptorList()));
            };

            auto inputs = std::make_shared<UTXODescriptorList>();
            static std::function<Future<Unit> (int, const std::shared_ptr<UTXODescriptorList>&, const std::shared_ptr<Buddy>&, const std::shared_ptr<BitcoinLikeUtxoPicker>&)> performFill
            = [] (int index, const std::shared_ptr<UTXODescriptorList>& inputs, const std::shared_ptr<Buddy>& buddy, const std::shared_ptr<BitcoinLikeUtxoPicker>& self) mutable -> Future<Unit> {
                if (index >= inputs->size())
                    return Future<Unit>::successful(unit);
                return self->fillInput(buddy, (*inputs)[index]).flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const Unit&) {
                    return performFill(index + 1, inputs, buddy, self);
                });
            };
            inputs->insert(inputs->end(), buddy->request.inputs.begin(), buddy->request.inputs.end());
            return performFill(0, inputs, buddy, self).flatMap<UTXODescriptorList>(ImmediateExecutionContext::INSTANCE, [=] (const Unit&) mutable -> Future<UTXODescriptorList> {
                return pickUtxo();
            }).flatMap<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const UTXODescriptorList& utxo) mutable -> Future<Unit> {
                auto offset = static_cast<int>(inputs->size());
                inputs->insert(inputs->end(), utxo.begin(), utxo.end());
                return performFill(offset, inputs, buddy, self);
            });
        }

        Future<Unit> BitcoinLikeUtxoPicker::fillInput(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                                      const BitcoinLikeUtxoPicker::UTXODescriptor &desc) {
            const std::string& hash = std::get<0>(desc);
            return buddy->explorer->getTransactionByHash(hash).map<Unit>(ImmediateExecutionContext::INSTANCE, [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::Transaction>& tx) {
                auto output = tx->outputs[std::get<1>(desc)];
                std::vector<std::vector<uint8_t>> pub_keys;
                std::vector<std::shared_ptr<api::DerivationPath>> paths;

                // Get derivations and public keys
                if (output.address.nonEmpty()) {
                    auto derivationPath = buddy->keychain->getAddressDerivationPath(output.address.getValue());
                    if (derivationPath.nonEmpty()) {
                        paths.push_back(std::make_shared<DerivationPathApi>(DerivationPath(derivationPath.getValue())));
                        pub_keys.push_back(buddy->keychain->getPublicKey(output.address.getValue()).getValue());
                    }
                }
                auto input = std::shared_ptr<BitcoinLikeWritableInputApi>(
                        new BitcoinLikeWritableInputApi(
                                buddy->explorer, getContext(), std::get<2>(desc), pub_keys, paths,
                                output.address.getValueOr(""),
                                std::make_shared<Amount>(buddy->keychain->getCurrency(), 0, output.value),
                                hash, std::get<1>(desc), {}, std::make_shared<BitcoinLikeOutputApi>(output, _currency)
                        )
                );
                buddy->transaction->addInput(input);
                return unit;
            });
        }

        BitcoinLikeGetUtxoFunction
        BitcoinLikeUtxoPicker::createFilteredUtxoFunction(const BitcoinLikeTransactionBuildRequest &request,
                                                          const BitcoinLikeGetUtxoFunction &getUtxo) {
            return [=] () -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                return getUtxo().map<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>(getContext(), [=] (const std::vector<std::shared_ptr<api::BitcoinLikeOutput>>& utxo) {
                    std::vector<std::shared_ptr<api::BitcoinLikeOutput>> filtered;
                    auto isExcluded = [&] (const std::shared_ptr<api::BitcoinLikeOutput>& output) -> bool {
                        for (auto& o : request.excludedUtxo) {
                            auto hash = std::get<0>(o);
                            auto index = std::get<1>(o);
                            if (output->getTransactionHash() == hash && output->getOutputIndex() == index)
                                return true;
                        }
                        return false;
                    };
                    for (auto& output : utxo) {
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