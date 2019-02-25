/*
 *
 * StellarLikeWallet.hpp
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

#ifndef LEDGER_CORE_STELLARLIKEWALLET_HPP
#define LEDGER_CORE_STELLARLIKEWALLET_HPP

#include <api/StellarLikeWallet.hpp>
#include <wallet/common/AbstractWallet.hpp>

namespace ledger {
    namespace core {

        struct StellarLikeWalletParams {

        };

        class StellarLikeWallet : public virtual api::StellarLikeWallet, public virtual AbstractWallet {
        public:
            static const api::WalletType type;

            StellarLikeWallet(  const std::string& walletName,
                                const api::Currency& currency,
                                const std::shared_ptr<WalletPool>& pool,
                                const std::shared_ptr<DynamicObject>& configuration,
                                const DerivationScheme& derivationScheme,
                                const StellarLikeWalletParams& params);

            bool isSynchronizing() override;

            std::shared_ptr<api::EventBus> synchronize() override;

            bool isInstanceOfStellarLikeWallet() override;

            std::shared_ptr<api::StellarLikeWallet> asStellarLikeWallet() override;

            FuturePtr<api::Account> newAccountWithInfo(const api::AccountCreationInfo &info) override;

            FuturePtr<api::Account>
            newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) override;

            Future<api::ExtendedKeyAccountCreationInfo>
            getExtendedKeyAccountCreationInfo(int32_t accountIndex) override;

            Future<api::AccountCreationInfo> getAccountCreationInfo(int32_t accountIndex) override;

        protected:
            std::shared_ptr<AbstractAccount>
            createAccountInstance(soci::session &sql, const std::string &accountUid) override;

        private:
            const StellarLikeWalletParams _params;
        };

    }
}

#endif //LEDGER_CORE_STELLARLIKEWALLET_HPP
