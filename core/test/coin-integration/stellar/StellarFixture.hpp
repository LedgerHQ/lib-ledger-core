/*
 *
 * StellarFixture.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/02/2019.
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

#ifndef LEDGER_CORE_STELLARFIXTURE_HPP
#define LEDGER_CORE_STELLARFIXTURE_HPP

#include "../common/CoinIntegrationFixture.hpp"
#include <wallet/stellar/StellarLikeWallet.hpp>
#include <wallet/stellar/StellarLikeAccount.hpp>
#include <wallet/stellar/StellarLikeOperation.hpp>

class StellarFixture : public CoinIntegrationFixture<StellarLikeWallet, StellarLikeAccount> {
public:
    std::shared_ptr<WalletPool> newPool(std::string poolName = "default_pool") override;

    api::AccountCreationInfo accountInfo(const std::string& pubKey) const;
    api::AccountCreationInfo defaultAccount() const;
    api::AccountCreationInfo emptyAccount() const;
    api::AccountCreationInfo accountInfoFromAddress(const std::string& address) const;

    api::Currency getCurrency() const;
};


#endif //LEDGER_CORE_STELLARFIXTURE_HPP
