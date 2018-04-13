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
                                                const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer,
                                                const std::shared_ptr<BitcoinLikeKeychain> &keychain) {
            auto self = shared_from_this();
            return [=] (const BitcoinLikeTransactionBuildRequest& r) -> Future<std::shared_ptr<api::BitcoinLikeTransaction>> {
                return self->async<std::shared_ptr<api::BitcoinLikeTransaction>>([=] () -> std::shared_ptr<api::BitcoinLikeTransaction> {
                    auto tx = std::make_shared<BitcoinLikeTransactionApi>(self->_currency);
                    Buddy buddy(r, explorer, keychain, *tx);
                    self->fillInputs(buddy);
                    self->fillOutputs(buddy);
                    self->fillTransactionInfo(buddy);
                    return tx;
                });
            };
        }

        const api::Currency &BitcoinLikeUtxoPicker::getCurrency() const {
            return _currency;
        }

        void BitcoinLikeUtxoPicker::fillOutputs(BitcoinLikeUtxoPicker::Buddy &buddy) {
            auto params = getCurrency().bitcoinLikeNetworkParameters.value();
            auto outputIndex = 0;
            for (auto &output : buddy.request.outputs) {
                auto amount = std::dynamic_pointer_cast<Amount>(std::get<0>(output))->value();
                auto script = std::dynamic_pointer_cast<BitcoinLikeScriptApi>(std::get<1>(output))->getScript();
                auto address = script.parseAddress(params).map<std::string>([] (const BitcoinLikeAddress& addr) {
                    return addr.toBase58();
                });
                BitcoinLikeBlockchainExplorer::Output out;
                out.index = outputIndex;
                out.value = *amount;
                out.address = address;
                out.script = hex::toString(script.serialize());
                std::shared_ptr<api::DerivationPath> derivationPath = nullptr;
                if (address.nonEmpty()) {
                    auto path = buddy.keychain->getAddressDerivationPath(out.address.getValue());
                    if (path.nonEmpty()) {
                        derivationPath = std::make_shared<DerivationPathApi>(DerivationPath(path.getValue()));
                    }
                }
                outputIndex += 1;
                buddy.transaction.addOutput(std::make_shared<BitcoinLikeOutputApi>(out, getCurrency(), derivationPath));
            }
        }

        void BitcoinLikeUtxoPicker::fillTransactionInfo(BitcoinLikeUtxoPicker::Buddy &buddy) {

        }
    }
}