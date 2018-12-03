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
#include <src/utils/optional.hpp>
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
        auto ethAddress = keychain.getAddress();
        EXPECT_EQ(ethAddress->toEIP55(), "0xE8F7Dc1A12F180d49c80D1c3DbEff48ee38bD1DA");
    });
}

TEST_F(EthereumKeychains, EthereumAddressValidation) {
    auto address = "0x8f7A0aFAAEE372EEFd020056FC552BD87DD75D73";
    auto ethAddress = ledger::core::EthereumLikeAddress::fromEIP55(address, ledger::core::currencies::ETHEREUM);
    EXPECT_EQ(ethAddress->toEIP55(), address);

}

TEST_F(EthereumKeychains, EthereumAddressValidationFromPubKeyAndChainCode) {
    auto address = "0xAc6603e97e774Cd34603293b69bBBB1980acEeaA";
    auto path = "44'/60'/0'";
    auto pubKey = "04d1dc4a3180fe2d56a1f02a68b053e59022ce5e107eae879ebef66a46d4ffe04dc3994facd376abcbab49c421599824a2600ee30e8520878e65581f598e2c497a";
    auto chainCode = "2d560fcaaedb929eea27d316dec7961eee884259e6483fdf192704db7582ca14";
    auto ethXpub = ledger::core::EthereumLikeExtendedPublicKey::fromRaw(ledger::core::currencies::ETHEREUM, optional<std::vector<uint8_t >>(), hex::toByteArray(pubKey), hex::toByteArray(chainCode), path);
    auto derive0 = ethXpub->derive("0");
    EXPECT_EQ(derive0->toEIP55(), address);

}