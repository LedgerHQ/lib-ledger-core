/*
 *
 * keychain_test_helper
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 08/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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
#include <src/wallet/ethereum/keychains/EthereumLikeKeychain.hpp>
#include <src/ethereum/EthereumLikeExtendedPublicKey.h>
#include <src/ethereum/EthereumLikeAddress.h>
#include "keychain_test_helper.h"
#include "../BaseFixture.h"
#include <iostream>
using namespace std;
class EthereumKeychains : public BaseFixture {
public:
    void testEthKeychain(const KeychainTestData &data, std::function<void (EthereumLikeKeychain&)> f) {
        auto backend = std::make_shared<ledger::core::PreferencesBackend>(
                "/preferences/tests.db",
                dispatcher->getMainExecutionContext(),
                resolver
        );
        auto configuration = std::make_shared<DynamicObject>();
        dispatcher->getMainExecutionContext()->execute(ledger::qt::make_runnable([=]() {
            EthereumLikeKeychain keychain(
                    configuration,
                    data.currency,
                    0,
                    ledger::core::EthereumLikeExtendedPublicKey::fromBase58(data.currency, data.xpub, optional<std::string>(data.derivationPath)),
                    backend->getPreferences("keychain")
            );
            f(keychain);
            dispatcher->stop();
        }));
        dispatcher->waitUntilStopped();
    };
};

TEST_F(EthereumKeychains, KeychainDerivation) {
    testEthKeychain(ETHEREUM_DATA, [] (EthereumLikeKeychain& keychain) {
        auto ethAddress = keychain.getFreshAddress();
        EXPECT_EQ(keychain.getFreshAddress()->toEIP55(), "0xE8F7Dc1A12F180d49c80D1c3DbEff48ee38bD1DA");
        auto addresses = keychain.getAllObservableAddresses(0, 5);
        EXPECT_EQ(addresses.size(), 6);
        EXPECT_EQ(addresses[0]->toEIP55(), "0xE8F7Dc1A12F180d49c80D1c3DbEff48ee38bD1DA");
        EXPECT_EQ(addresses[1]->toEIP55(), "0xEC9eD3b9489735992B3c48DE9F3b6bD076eC3bfA");
        EXPECT_EQ(addresses[2]->toEIP55(), "0x67C236376fbB0FBC3662aFdf78d3A3D46F756306");
        EXPECT_EQ(addresses[3]->toEIP55(), "0x1aED60053943aD2eDEA0dd9652CBF4e783D1bdA3");
        EXPECT_EQ(addresses[4]->toEIP55(), "0xF503aE5520B1315BA575393FFA61B413C77Ba45d");
        EXPECT_EQ(addresses[5]->toEIP55(), "0x6965130E8687767ABA43911846122EC3293429dF");
    });
}

TEST_F(EthereumKeychains, EthereumAddressValidation) {
    auto address = "0x8f7A0aFAAEE372EEFd020056FC552BD87DD75D73";
    auto ethAddress = ledger::core::EthereumLikeAddress::fromEIP55(address, ledger::core::currencies::ETHEREUM);
    EXPECT_EQ(ethAddress->toEIP55(), address);

}