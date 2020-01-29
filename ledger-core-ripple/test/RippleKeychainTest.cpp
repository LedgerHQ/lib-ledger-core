/*
 *
 * ripple_keychain_test
 *
 * Created by El Khalil Bellakrid on 22/07/2019.
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

#include <gtest/gtest.h>

#include <core/utils/DerivationPath.hpp>
#include <core/utils/Optional.hpp>
#include <ripple/keychains/RippleLikeKeychain.hpp>
#include <ripple/RippleLikeExtendedPublicKey.hpp>
#include <ripple/RippleLikeAccount.hpp>
#include <ripple/RippleLikeAddress.hpp>
#include <ripple/RippleLikeCurrencies.hpp>
#include <ripple/RippleLikeWallet.hpp>
#include <ripple/factories/RippleLikeWalletFactory.hpp>

#include <integration/BaseFixture.hpp>

struct RippleKeychains : BaseFixture {

};

struct DerivationSchemeData {
    std::vector<std::string> equivalentDerivationSchemes;
    std::string expectedDerivationPath;
    std::string pubKey;
    std::string chainCode;
    std::string expectedAddress;
};

// Data taken from nanoS of dev
const std::vector<DerivationSchemeData> derivationSchemeTestData = {
        {
                {"44'/<coin_type>'/<account>'/<node>/<address>", "44'/<coin_type>'/<account>'/<node>/0", "44'/<coin_type>'/<account>'/0/0"},
                "44'/144'/0'",
                "03c73f64083463fa923e1530af6f558204853873c6a45cbfb1f2f1e2ac2a5d989c",
                "f7e8d16154d3c7cbfa2cea35aa7a6ae0c429980892cf2d6ea9e031f57f22a63d",
                "rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7"
        },
        {
                {"44'/144'/0'/0'"},
                "44'/144'/0'/0'",
                "0274282fa4d2f1e58f8a5735dc6acaa1b0b56c20bbfe368e66f3803cc015c9acf5",
                "70fe2dbddb85ba2656043f0732b7ea6f24f226c9a243daa0d9283b969e988af6",
                "rUsnp1vzpoUCeDRG9hmrTtScrdffT7Yx52"
        },
        {
                {"44'/144'/14'/5'/16"},
                "44'/144'/14'/5'",
                "02ea8289915225abedc9a24239ddfad883d7f3b30205e0a1178dca1c32bdbd3b32",
                "6a4ecb12b1b59d08d21b59f6a28e6ea2bdf7f03c08c8b4a9adb7cc988df275a1",
                "rLJEcapF3TyMgwE8wmBvj4gYiZrcTdf3it"
        }
};

TEST_F(RippleKeychains, RippleDerivationSchemes) {
    using namespace std;

    auto services = newDefaultServices();
    auto configuration = DynamicObject::newInstance();

    auto walletStore = newWalletStore(services);
    wait(walletStore->addCurrency(currencies::ripple()));

    auto factory = std::make_shared<RippleLikeWalletFactory>(currencies::ripple(), services);
    walletStore->registerFactory(currencies::ripple(), factory);

    {
        for (auto &elem : derivationSchemeTestData) {
            auto derivationSchemes = elem.equivalentDerivationSchemes;
            for (auto &scheme : derivationSchemes) {
                configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, scheme);

                auto wallet = std::dynamic_pointer_cast<RippleLikeWallet>(wait(walletStore->createWallet(scheme, "ripple", configuration)));

                //Create account as Live does
                api::AccountCreationInfo info = wait(wallet->getNextAccountCreationInfo());
                EXPECT_EQ(info.derivations[0], elem.expectedDerivationPath);
                info.publicKeys.push_back(hex::toByteArray(elem.pubKey));
                info.chainCodes.push_back(hex::toByteArray(elem.chainCode));

                auto account = std::dynamic_pointer_cast<RippleLikeAccount>(wait(wallet->newAccountWithInfo(info)));
                auto addresses = wait(account->getFreshPublicAddresses());
                EXPECT_GT(addresses.size(), 0);
                EXPECT_EQ(addresses[0]->toString(), elem.expectedAddress);
            }
        }
    }
}
