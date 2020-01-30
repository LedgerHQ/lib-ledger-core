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

#include <core/api/ErrorCode.hpp>
#include <core/api/ExtendedKeyAccountCreationInfo.hpp>

#include <ethereum/EthereumLikeAccount.hpp>
#include <ethereum/EthereumLikeCurrencies.hpp>
#include <ethereum/EthereumLikeWallet.hpp>
#include <ethereum/factories/EthereumLikeWalletFactory.hpp>

#include <integration/WalletFixture.hpp>

namespace {

constexpr auto const OLD_NODE_ENDPOINT = "http://eth-ropsten.explorers.dev.aws.ledger.fr";
constexpr auto const NEW_NODE_ENDPOINT = "http://eth-ropsten.explorers.prod.aws.ledger.fr";

} // namespace

class EthereumWallets : public WalletFixture<EthereumLikeWalletFactory> {

};

TEST_F(EthereumWallets, ChangeWalletConfig) {
    auto currency = currencies::ethereum();
    auto derivationScheme = "44'/<coin_type>'/<account>'/<node>/<address>";
    
    {
        auto configuration = api::DynamicObject::newInstance();

        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, OLD_NODE_ENDPOINT);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, derivationScheme);
        
        testWallet("ethereum", currency, TestStrategy::NEW_WALLET, configuration, [&derivationScheme](auto wallet, auto walletStore) {
            auto configuration = wallet->getConfiguration();
            
            EXPECT_EQ(configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT).value(), OLD_NODE_ENDPOINT);
            EXPECT_EQ(configuration->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME).value(), derivationScheme); 
        });
    }

    {
        auto configuration = api::DynamicObject::newInstance();

        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, NEW_NODE_ENDPOINT);

        testWallet("ethereum", currency, TestStrategy::EXISTING_WALLET, configuration, [&derivationScheme](auto wallet, auto walletStore) {
            auto configuration = wallet->getConfiguration();
            
            EXPECT_EQ(configuration->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT).value(), NEW_NODE_ENDPOINT);
            EXPECT_EQ(configuration->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME).value(), derivationScheme);  
        });
    }
}