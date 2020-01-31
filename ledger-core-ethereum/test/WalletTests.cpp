/*
 *
 * WalletTests
 * ledger-core-ethereum
 *
 * Created by Alexis Le Provost on 30/01/2020.
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

#include <gtest/gtest.h>

#include <core/api/ErrorCode.hpp>

#include <ethereum/EthereumLikeCurrencies.hpp>
#include <ethereum/factories/EthereumLikeWalletFactory.hpp>

#include <integration/WalletFixture.hpp>

class EthereumWallets : public WalletFixture<EthereumLikeWalletFactory> {

};

TEST_F(EthereumWallets, ChangeWalletConfig) {
    auto const oldEndpoint = "http://eth-ropsten.explorers.dev.aws.ledger.fr";
    auto const newEndpoint = "http://eth-ropsten.explorers.prod.aws.ledger.fr";
    auto const derivationScheme = "44'/<coin_type>'/<account>'/<node>/<address>";
    auto const walletName = "ethereum";
    auto const currency = currencies::ethereum();

    registerCurrency(currency);
    
    {
        auto configuration = api::DynamicObject::newInstance();

        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, oldEndpoint);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, derivationScheme);
        
        auto wallet = wait(walletStore->createWallet(walletName, currency.name, configuration));
        auto walletConfiguration = wallet->getConfiguration();
            
        EXPECT_EQ(walletConfiguration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT).value(), oldEndpoint);
        EXPECT_EQ(walletConfiguration->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME).value(), derivationScheme);
    }

    {
        auto configuration = api::DynamicObject::newInstance();

        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, newEndpoint);

        EXPECT_EQ(wait(walletStore->updateWalletConfig(walletName, configuration)), api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
        
        auto wallet = wait(walletStore->getWallet(walletName));
        auto walletConfiguration = wallet->getConfiguration();
            
        EXPECT_EQ(walletConfiguration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT).value(), newEndpoint);
        EXPECT_EQ(walletConfiguration->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME).value(), derivationScheme);  
    }
}