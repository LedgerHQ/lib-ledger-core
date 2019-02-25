/*
 *
 * StellarLikeWalletFactory.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/02/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include "StellarLikeWalletFactory.hpp"
#include <wallet/stellar/StellarLikeWallet.hpp>
#include <wallet/pool/WalletPool.hpp>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {

        StellarLikeWalletFactory::StellarLikeWalletFactory(const api::Currency &currency,
                                                           const std::shared_ptr<WalletPool> &pool)
                : AbstractWalletFactory(currency, pool) {

        }

        std::shared_ptr<AbstractWallet> StellarLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
            StellarLikeWalletParams params;
            auto currency = getPool()->getCurrency(entry.currencyName).getOrElse([&] () -> api::Currency {
                throw make_exception(api::ErrorCode::CURRENCY_NOT_FOUND, "Currency '{}' not found", entry.currencyName);
            });

            // Get derivation scheme
            DerivationScheme scheme(STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'"));

            // Configure keychain factory

            // Configure explorer

            // Configure observer

            // Configure synchronizer

            return std::make_shared<StellarLikeWallet>(entry.name, currency, getPool(), entry.configuration, scheme, params);
        }

    }
}
