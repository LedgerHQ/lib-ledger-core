/*
 *
 * BitcoinLikeP2PKHKeychainFactory
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/06/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include <async/Promise.hpp>
#include "BitcoinLikeP2PKHKeychainFactory.h"
#include <wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>

namespace ledger {
    namespace core {

        FuturePtr<ledger::core::BitcoinLikeKeychain>
        BitcoinLikeP2PKHKeychainFactory::build(const std::shared_ptr<api::ExecutionContext> &context,
                                               int32_t index,
                                               const DerivationPath &path,
                                               const std::shared_ptr<DynamicObject> &configuration,
                                               const std::shared_ptr<BitcoinLikeExtendedPublicKeyProvider> &provider,
                                               const std::shared_ptr<Preferences> &accountPreferences,
                                               const api::Currency &currency) {
            return provider->get(context,
                          "main",
                          path.toString(),
                          currency.bitcoinLikeNetworkParameters.value()
            ).mapPtr<BitcoinLikeKeychain>(context, [=] (const std::shared_ptr<BitcoinLikeExtendedPublicKey>& xpub) {
                return std::make_shared<P2PKHBitcoinLikeKeychain>(
                        configuration, currency, index, xpub, accountPreferences
                );
            });
        }

        FuturePtr<ledger::core::BitcoinLikeKeychain>
        BitcoinLikeP2PKHKeychainFactory::restore(const std::shared_ptr<api::ExecutionContext> &context,
                                                 int32_t index,
                                                 const DerivationPath &path,
                                                 const std::shared_ptr<DynamicObject> &configuration,
                                                 const std::string &databaseXpubEntry,
                                                 const std::shared_ptr<Preferences> &accountPreferences,
                                                 const api::Currency &currency) {
            PromisePtr<BitcoinLikeKeychain> promise;

            return promise.getFuture();
        }
    }
}