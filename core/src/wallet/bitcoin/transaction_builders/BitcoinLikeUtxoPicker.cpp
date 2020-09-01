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
                                                const uint64_t currentBlockHeight,
                                                const std::shared_ptr<spdlog::logger>& logger,
                                                bool partial)
        {
            auto self = shared_from_this();
            logger->info("Get build function");
            return [=] (const BitcoinLikeTransactionBuildRequest& r) -> Future<std::shared_ptr<api::BitcoinLikeTransaction>> {
                return self->async<std::shared_ptr<Buddy>>([=] () {
                    logger->info("Constructing BitcoinLikeTransactionBuildFunction with blockHeight: {}", currentBlockHeight);
                    auto tx = std::make_shared<BitcoinLikeTransactionApi>(self->_currency, keychain->getKeychainEngine(), currentBlockHeight);
                    auto filteredGetUtxo = createFilteredUtxoFunction(r, keychain, getUtxo);
                    return std::make_shared<Buddy>(r, filteredGetUtxo, getTransaction, explorer, keychain, logger, tx, partial);
                }).flatMap<std::shared_ptr<api::BitcoinLikeTransaction>>(self->getContext(), [=] (const std::shared_ptr<Buddy>& buddy) -> Future<std::shared_ptr<api::BitcoinLikeTransaction>> {
                    buddy->logger->info("Buddy created");
                    return self->fillInputs(buddy).flatMap<Unit>(self->getContext(), [=] (const Unit&) -> Future<Unit> {
                        return self->fillOutputs(buddy);
                    }).flatMap<Unit>(self->getContext(), [=] (const Unit&) -> Future<Unit> {
                        return self->fillTransactionInfo(buddy);
                    }).mapPtr<api::BitcoinLikeTransaction>(self->getContext(), [=] (const Unit&) -> std::shared_ptr<api::BitcoinLikeTransaction> {
                        buddy->logger->info("Empty transaction built");
                        return buddy->transaction;
                    });
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
                //Set right amount if we are in wipe mode
                if(buddy->request.wipe) {
                    amount = std::make_shared<ledger::core::BigInt>(buddy->outputAmount);
                }
                auto script = std::dynamic_pointer_cast<BitcoinLikeScriptApi>(std::get<1>(output))->getScript();
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
                        derivationPath = std::make_shared<DerivationPathApi>(DerivationPath(path.getValue()));
                    }
                } else {
                    auto addressFromScript = script.parseAddress(getCurrency());
                    if (addressFromScript.nonEmpty())
                        out.address = addressFromScript.getValue().toString();
                }
                outputIndex += 1;
                buddy->transaction->addOutput(std::make_shared<BitcoinLikeOutputApi>(out, getCurrency(), derivationPath));
            }

            // Fill change outputs

            if (buddy->changeAmount > BigInt(_currency.bitcoinLikeNetworkParameters.value().DustAmount)) {
                // TODO implement multi change
                std::string changeAddress;
                if (buddy->request.changePaths.size() != 0)
                {
                    auto changePath = buddy->request.changePaths.front();
                    std::shared_ptr<const CommonBitcoinLikeKeychains> buddy_keychain = std::static_pointer_cast<CommonBitcoinLikeKeychains>(buddy->keychain);
                    auto localPath = buddy_keychain->getDerivationScheme().getSchemeTo(DerivationSchemeLevel::NODE)
                        .setAccountIndex(buddy_keychain->getAccountIndex())
                        .setCoinType(getCurrency().bip44CoinType)
                        .setNode(BitcoinLikeKeychain::CHANGE).getPath();
                    auto internalNodeXpub = std::static_pointer_cast<BitcoinLikeExtendedPublicKey>(buddy_keychain->getExtendedPublicKey())->derive(localPath);
                    changeAddress = BitcoinLikeAddress::fromPublicKey(internalNodeXpub, _currency, changePath, buddy_keychain->getKeychainEngine());
                }
                else
                    changeAddress = buddy->keychain->getFreshAddress(BitcoinLikeKeychain::CHANGE)->toString();

                auto amount = buddy->changeAmount;
                auto script = BitcoinLikeScript::fromAddress(changeAddress, _currency);
                BitcoinLikeBlockchainExplorerOutput out;
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
            if (buddy->isPartial) {
                return Future<Unit>::successful(unit);
            }
            //Set timestamp
            buddy->explorer->getTimestamp().onComplete(getContext(), [=] (const Try<int64_t> &timestamp){
                if (timestamp.isSuccess()) {
                    buddy->transaction->setTimestamp(timestamp.getValue());
                }
            });

            return buddy->explorer->getCurrentBlock().map<Unit>(getContext(), [=] (const std::shared_ptr<BitcoinLikeBlockchainExplorer::Block>& block) -> Unit{
                buddy->transaction->setLockTime(static_cast<uint32_t>(block->height));
                return unit;
            });
        }

        Future<Unit> BitcoinLikeUtxoPicker::fillInputs(const std::shared_ptr<Buddy>& buddy) {
            buddy->logger->info("Filling inputs");

            auto self = shared_from_this();

            auto performFill = [self, buddy](auto performFill, auto index)
            {
                if (index >= buddy->request.inputs.size()) {
                    return Future<Unit>::successful(unit);
                }

                auto const &input = buddy->request.inputs[index];

                return buddy->getTransaction(input.transactionHash).template flatMap<Unit>(self->getContext(), [=](auto const &tx) {
                    auto const utxo = makeUtxo(tx->outputs[input.outputIndex], self->getCurrency());

                    self->fillInput(buddy, utxo, input.sequence);

                    return performFill(performFill, index + 1);
                });
            };

            // first fill inputs from user-defined input descriptors
            return performFill(performFill, 0)
                .filter(getContext(), [buddy](auto const&) {
                    return buddy->request.utxoPicker.nonEmpty();
                })
                .template flatMap<std::vector<BitcoinLikeUtxo>>(getContext(), [self, buddy](auto const&) {
                    return self->filterInputs(buddy);
                })
                .template flatMap<Unit>(getContext(), [self, buddy] (auto const& utxos) mutable {
                    auto const sequence = std::get<1>(buddy->request.utxoPicker.getValue());

                    std::for_each(utxos.cbegin(), utxos.cend(), [self, buddy, sequence] (auto const &utxo) {
                        self->fillInput(buddy, utxo, sequence);
                    });

                    return Future<Unit>::successful(unit);
                });
        }

        void BitcoinLikeUtxoPicker::fillInput(const std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> &buddy,
                                              const BitcoinLikeUtxo &utxo,
                                              const uint32_t sequence) {
            std::vector<std::vector<uint8_t>> pub_keys;
            std::vector<std::shared_ptr<api::DerivationPath>> paths;

            // Get derivations and public keys
            if (utxo.address.nonEmpty()) {
                auto const derivationPath = buddy->keychain->getAddressDerivationPath(utxo.address.getValue());

                if (derivationPath.nonEmpty()) {
                    paths.push_back(std::make_shared<DerivationPathApi>(DerivationPath(derivationPath.getValue())));

                    pub_keys.push_back(buddy->keychain->getPublicKey(utxo.address.getValue()).getValue());
                }

                auto input = std::shared_ptr<BitcoinLikeWritableInputApi>(
                        new BitcoinLikeWritableInputApi(
                            buddy->explorer,
                            getContext(),
                            sequence,
                            pub_keys,
                            paths,
                            utxo.address.getValueOr(""),
                            // NOTE: we previously used buddy->keychain->getCurrency() which is weird since
                            // we can get that information from current object from _currency attribute
                            std::make_shared<Amount>(utxo.value),
                            utxo.transactionHash,
                            utxo.index,
                            {},
                            std::make_shared<BitcoinLikeOutputApi>(toExplorerOutput(utxo), _currency)
                        )
                );
                buddy->transaction->addInput(input);
            }
        }

        BitcoinLikeGetUtxoFunction
        BitcoinLikeUtxoPicker::createFilteredUtxoFunction(const BitcoinLikeTransactionBuildRequest &request,
                                                          const std::shared_ptr<BitcoinLikeKeychain> &keychain,
                                                          const BitcoinLikeGetUtxoFunction &getUtxo) {
            auto minAmount = getCurrency().bitcoinLikeNetworkParameters.value().DustAmount;
            return [=] () -> Future<std::vector<BitcoinLikeUtxo>> {
                return getUtxo().map<std::vector<BitcoinLikeUtxo>>(getContext(), [=] (auto const &utxos) {
                    auto const isNotExcluded = [&] (auto const &currentUtxo) {
                        // NOTE: This logic can be move to the SQL request itself but since we iterate through
                        // the entire vector to compare the UTXO amount, this is fine to apply the filter(s) here.
                        // The filter are sorted by ascending time order.
                        return !(currentUtxo.address.isEmpty()
                                 || currentUtxo.value.toLong() < minAmount
                                 || !keychain->contains(currentUtxo.address.getValue())
                                 || request.excludedUtxos.count(BitcoinLikeTransactionUtxoDescriptor{currentUtxo.transactionHash, currentUtxo.index}) > 0);
                    };

                    std::vector<BitcoinLikeUtxo> filteredUtxos;

                    std::copy_if(utxos.begin(), utxos.end(), std::back_inserter(filteredUtxos), isNotExcluded);

                    return filteredUtxos;
                });
            };
        }
    }
}
