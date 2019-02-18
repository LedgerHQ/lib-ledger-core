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
#include <ledger/core/api/BitcoinLikeNetworkParameters.hpp>
#include <ledger/core/utils/hex.h>
#include <ledger/core/api/BitcoinLikeExtendedPublicKey.hpp>
#include <ledger/core/utils/optional.hpp>
#include <ledger/core/api/Networks.hpp>
#include <wallet/currencies.hpp>
#include <api/Address.hpp>
#include <bitcoin/BitcoinLikeExtendedPublicKey.hpp>


std::vector<std::vector<std::string>> fixtures = {
        {"010966776006953D5567439E5E39F86A0D273BEE", "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM", "00"},
        {"7b2f2061d66d57ffb9502a091ce236ed4c1ede2d", "1CELa15H4DMzHtHnuz7LCpSFgFWf61Ra6A", "00"},
        {"89C907892A9D4F37B78D5F83F2FD6E008C4F795D", "1DZYQ3xEy8mkc7wToQZvKqeLrSLUMVVK41", "00"},
        {"0000000000000000000000000000000000000000", "1111111111111111111114oLvT2", "00"},
        {"0000000000000000000000000000000000000001", "11111111111111111111BZbvjr", "00"},
        {"dd894dfc0f473c44cc983b5dd462bc1b393f7498", "3MtPk2kf61VepUfwgRjqjC5WeGgxE4rRPS", "05"}
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
        EXPECT_EQ(address->getVersion(), hex::toByteArray(item[2]));
        if (item[2] == "05") {
            EXPECT_TRUE(address->isP2SH());
        } else {
            EXPECT_TRUE(address->isP2PKH());
        }
        if (item == fixtures.back()) {
            EXPECT_EQ(address->toBech32(), "bc19mky5mlq0gu7yfnyc8dwagc4urvun7ayc940jpl");
        }
    }
}


TEST(Address, XpubFromBase58String) {
    const Currency currency = currencies::BITCOIN;
    auto addr = "xpub6Cc939fyHvfB9pPLWd3bSyyQFvgKbwhidca49jGCM5Hz5ypEPGf9JVXB4NBuUfPgoHnMjN6oNgdC9KRqM11RZtL8QLW6rFKziNwHDYhZ6Kx";
    auto xpub = ledger::core::BitcoinLikeExtendedPublicKey::fromBase58(currency, addr, optional<std::string>("44'/0'/0'"));
    EXPECT_EQ(xpub->toBase58(), addr);
    EXPECT_EQ(xpub->derive("0/0")->toBase58(), "14NjenDKkGGq1McUgoSkeUHJpW3rrKLbPW");
    EXPECT_EQ(xpub->derive("0/1")->toBase58(), "1Pn6i3cvdGhqbdgNjXHfbaYfiuviPiymXj");
    EXPECT_EQ(xpub->derive("1/0")->toBase58(), "17HHBbhmF324wBw8Fo6tJVVriedJ6mFum8");
    EXPECT_EQ(xpub->derive("1/1")->toBase58(), "1AkRBkUZQe5Zqj5syxn1cHCvKUV6DjL9Po");
}

TEST(Address, XpubFromBase58StringToBech32) {
    const Currency currency = currencies::BITCOIN_CASH;
    auto xpubStr = "xpub6BvNdfGcyMB9Usq88ibXUt3KhbaEJVLFMbhTSNNfTm8Qf1sX9inTv3xL6pA6KofW4WF9GpdxwGDoYRwRDjHEir3Av23m2wHb7AqhxJ9ohE8";
    auto xpub = ledger::core::BitcoinLikeExtendedPublicKey::fromBase58(currency, xpubStr, optional<std::string>("49'/145'/0'"));
    EXPECT_EQ(xpub->toBase58(), xpubStr);
    EXPECT_EQ(xpub->derive("0/0")->toBase58(), "16AMaKewP778obhBUAWWV5sVU6Qg6rvuBt");
    EXPECT_EQ(xpub->derive("0/0")->toBech32(), "bitcoincash:qqufmrqunkr3avkswhn378fjhwl3ueawag9e3htc49");
}