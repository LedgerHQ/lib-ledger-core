/*
 *
 * StellarLikeWallet.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
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

#include "StellarLikeWallet.hpp"

namespace ledger {
    namespace core {

        const api::WalletType StellarLikeWallet::type = api::WalletType::STELLAR;

        StellarLikeWallet::StellarLikeWallet(const std::string &walletName, const api::Currency &currency,
                                             const std::shared_ptr<WalletPool> &pool,
                                             const std::shared_ptr<DynamicObject> &configuration,
                                             const DerivationScheme &derivationScheme,
                                             const StellarLikeWalletParams &params) :
                                             AbstractWallet(walletName, currency, pool, configuration, derivationScheme),
                                             _params(params) {

        }


        bool StellarLikeWallet::isSynchronizing() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::shared_ptr<api::EventBus> StellarLikeWallet::synchronize() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        bool StellarLikeWallet::isInstanceOfStellarLikeWallet() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::shared_ptr<api::StellarLikeWallet> StellarLikeWallet::asStellarLikeWallet() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        FuturePtr<api::Account>
        StellarLikeWallet::newAccountWithInfo(const api::AccountCreationInfo &info) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        FuturePtr<ledger::core::api::Account>
        StellarLikeWallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "StellarLike doesn't support account creation through extended key info.");
        }

        Future<api::ExtendedKeyAccountCreationInfo>
        StellarLikeWallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION,
                                 "StellarLike doesn't support account creation through extended key info.");
        }

        Future<api::AccountCreationInfo> StellarLikeWallet::getAccountCreationInfo(int32_t accountIndex) {
            auto scheme = getDerivationScheme();
            auto path = scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(accountIndex).getPath();
            api::AccountCreationInfo info {accountIndex, {"main"}, {path.toString()}, {}, {}};
            return Future<api::AccountCreationInfo>::successful(info);
        }

        std::shared_ptr<AbstractAccount>
        StellarLikeWallet::createAccountInstance(soci::session &sql, const std::string &accountUid) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

    }
}