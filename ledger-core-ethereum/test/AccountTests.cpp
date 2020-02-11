/*
 *
 * AccountTests
 * ledger-core-ethereum
 *
 * Created by El Khalil Bellakrid on 11/02/202.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include <ethereum/EthereumLikeCurrencies.hpp>
#include <ethereum/EthereumLikeWallet.hpp>
#include <ethereum/factories/EthereumLikeWalletFactory.hpp>

#include <integration/WalletFixture.hpp>


struct EthereumAccounts : public WalletFixture<EthereumLikeWalletFactory> {

};


TEST_F(EthereumAccounts, FirstEthAccountInfo) {
    auto const currency = currencies::ethereum();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("my_wallet", currency.name, api::DynamicObject::newInstance()));
    auto info = wait(wallet->getNextAccountCreationInfo());
    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    // TODO: info.derivations[0] is equal to "44'/61'/0'". Is it a regression ?
    EXPECT_EQ(info.derivations[0], "44'/60'/0'");
}