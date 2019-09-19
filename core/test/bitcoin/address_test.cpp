/*
 *
 * address_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include <ledger/core/api/BitcoinLikeAddress.hpp>
#include <ledger/core/bitcoin/BitcoinLikeAddress.hpp>
#include <ledger/core/api/BitcoinLikeNetworkParameters.hpp>
#include <ledger/core/utils/hex.h>
#include <ledger/core/api/BitcoinLikeExtendedPublicKey.hpp>
#include <ledger/core/utils/optional.hpp>
#include <ledger/core/api/Networks.hpp>
#include <ledger/core/collections/DynamicObject.hpp>
#include <wallet/currencies.hpp>
#include <api/Address.hpp>
#include <bitcoin/BitcoinLikeExtendedPublicKey.hpp>
#include <api/Configuration.hpp>
#include <api/KeychainEngines.hpp>


std::vector<std::vector<std::string>> fixtures = {
        {"010966776006953D5567439E5E39F86A0D273BEE", "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM", "bc1qqyykvamqq62n64t8gw09uw0cdgxjwwlw7mypam", "00"},
        {"7b2f2061d66d57ffb9502a091ce236ed4c1ede2d", "1CELa15H4DMzHtHnuz7LCpSFgFWf61Ra6A", "bc1q0vhjqcwkd4tllw2s9gy3ec3ka4xpah3dphtgsd", "00"},
        {"89C907892A9D4F37B78D5F83F2FD6E008C4F795D", "1DZYQ3xEy8mkc7wToQZvKqeLrSLUMVVK41", "bc1q38ys0zf2n48n0dudt7pl9ltwqzxy772af9ng0d", "00"},
        {"0000000000000000000000000000000000000000", "1111111111111111111114oLvT2", "bc1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq9e75rs", "00"},
        {"0000000000000000000000000000000000000001", "11111111111111111111BZbvjr", "bc1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqpc02p7z", "00"},
        {"dd894dfc0f473c44cc983b5dd462bc1b393f7498", "3MtPk2kf61VepUfwgRjqjC5WeGgxE4rRPS", "bc1qmky5mlq0gu7yfnyc8dwagc4urvun7ayctgku33", "05"}
};

using namespace ledger::core::api;
using namespace ledger::core;


TEST(Address, AddressFromBase58String) {
    const Currency currency = currencies::BITCOIN;
    auto x = currency.bitcoinLikeNetworkParameters.value();
    for (auto& item : fixtures) {
        EXPECT_TRUE(Address::isValid(item[1], currency));
        auto address = Address::parse(item[1], currency)->asBitcoinLikeAddress();
        EXPECT_EQ(address->getHash160(), hex::toByteArray(item[0]));
        EXPECT_EQ(address->getVersion(), hex::toByteArray(item[3]));
        if (item[3] == "05") {
            EXPECT_TRUE(address->isP2SH());
        } else {
            EXPECT_TRUE(address->isP2PKH());
        }
        EXPECT_EQ(address->toBech32(), item[2]);
    }
}


TEST(Address, XpubFromBase58String) {
    const Currency currency = currencies::BITCOIN;
    auto addr = "xpub6Cc939fyHvfB9pPLWd3bSyyQFvgKbwhidca49jGCM5Hz5ypEPGf9JVXB4NBuUfPgoHnMjN6oNgdC9KRqM11RZtL8QLW6rFKziNwHDYhZ6Kx";
    auto config = std::make_shared<ledger::core::DynamicObject>();
    config->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP32_P2PKH);
    auto xpub = ledger::core::BitcoinLikeExtendedPublicKey::fromBase58(currency, addr, optional<std::string>("44'/0'/0'"), config);
    EXPECT_EQ(xpub->toBase58(), addr);
    EXPECT_EQ(xpub->derive("0/0")->toBase58(), "14NjenDKkGGq1McUgoSkeUHJpW3rrKLbPW");
    EXPECT_EQ(xpub->derive("0/1")->toBase58(), "1Pn6i3cvdGhqbdgNjXHfbaYfiuviPiymXj");
    EXPECT_EQ(xpub->derive("1/0")->toBase58(), "17HHBbhmF324wBw8Fo6tJVVriedJ6mFum8");
    EXPECT_EQ(xpub->derive("1/1")->toBase58(), "1AkRBkUZQe5Zqj5syxn1cHCvKUV6DjL9Po");
}

TEST(Address, XpubFromBase58StringToBech32) {
    const Currency currency = currencies::BITCOIN_CASH;
    auto xpubStr = "xpub6BvNdfGcyMB9Usq88ibXUt3KhbaEJVLFMbhTSNNfTm8Qf1sX9inTv3xL6pA6KofW4WF9GpdxwGDoYRwRDjHEir3Av23m2wHb7AqhxJ9ohE8";
    auto base58Address = "16AMaKewP778obhBUAWWV5sVU6Qg6rvuBt";
    auto bech32Address = "bitcoincash:qqufmrqunkr3avkswhn378fjhwl3ueawag9e3htc49";
    auto config = std::make_shared<ledger::core::DynamicObject>();
    config->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP173_P2WPKH);
    auto xpub = ledger::core::BitcoinLikeExtendedPublicKey::fromBase58(currency, xpubStr, optional<std::string>("49'/145'/0'"), config);
    EXPECT_EQ(xpub->toBase58(), xpubStr);
    EXPECT_EQ(xpub->derive("0/0")->toBase58(), base58Address);
    EXPECT_EQ(xpub->derive("0/0")->toBech32(), bech32Address);

    auto addr = ledger::core::BitcoinLikeAddress::fromBech32(bech32Address, currency);
    EXPECT_EQ(addr->toBase58(), base58Address);
}

TEST(Address, FromBech32Address) {
    //https://github.com/bitcoincashjs/cashaddrjs/blob/master/test/cashaddr.js
    std::vector<std::pair<std::string, ledger::core::api::Currency>> tests = {
            {"tb1qunawpra24prfc46klknlhl0ydy32feajmwpg84", currencies::BITCOIN_TESTNET},//BTC P2WPKH
            {"bc1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3qccfmv3", currencies::BITCOIN},//BTC P2WSH
            {"bitcoincash:qpm2qsznhks23z7629mms6s4cwef74vcwvy22gdx6a", currencies::BITCOIN_CASH},//BCH P2WPKH
            {"bitcoincash:ppm2qsznhks23z7629mms6s4cwef74vcwvn0h829pq", currencies::BITCOIN_CASH}//BCH P2WSH
    };
    for (auto &test : tests) {
        auto address = ledger::core::BitcoinLikeAddress::fromBech32(test.first, test.second);
        EXPECT_EQ(address->toBech32(), test.first);
    }
}

TEST(Address, CreateXpub) {
    auto conf = std::make_shared<ledger::core::DynamicObject>();

    auto liveData = Base58::decode("xpub6BezZZ1HGWH3iGguzANbj6zLM4hqjGcpPUtTTwmSUc2DSEybcHSn5BUp5pVH562sWQPCh8qgNZE1NKB4RYL2aEReu4kxBCEYf5nXvsERbxW", conf);
    auto myData = Base58::decode("xpub6BezZZ19T52Wo8fgtVNT87SzZ9qq3sRbhEjtE4F7fRx1XCiuaFfqgbrGsxtmNiNhbA5BSz4amDm52YSMdR2WaeksKAMuZE744EjiAUiVsEB", conf);
    int k = liveData.size() + myData.size();
}