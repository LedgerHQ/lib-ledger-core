/*
 *
 * address_test
 *
 * Created by El Khalil Bellakrid on 05/01/2019.
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

#include <core/api/Address.hpp>
#include <core/utils/Hex.hpp>
#include <core/utils/Optional.hpp>
#include <core/api/Currency.hpp>

#include <ripple/api/RippleLikeExtendedPublicKey.hpp>
#include <ripple/RippleLikeAddress.hpp>
#include <ripple/RippleLikeExtendedPublicKey.hpp>
#include <ripple/RippleLikeCurrencies.hpp>

using namespace ledger::core::api;
using namespace ledger::core;

Currency currency() {
  static Currency currency = currencies::RIPPLE;
  return currency;
}

TEST(RippleAddress, AddressFromPubKeyAndChainCodeAccountLevel) {
    std::vector<uint8_t> pubKey = hex::toByteArray(
            "039ea82278f0057b8b041a146f810aa2d84b8ffdfaef3e625624706ad13e12b717");
    std::vector<uint8_t> chainCode = hex::toByteArray(
            "abcc4933bec06eeca6628b9e44f8e71d5e3cf510c0450dd1e29d9aa0f1717da9");
    auto xpub = ledger::core::RippleLikeExtendedPublicKey::fromRaw(currency(),
                                                                   optional<std::vector<uint8_t >>(),
                                                                   pubKey,
                                                                   chainCode,
                                                                   "44'/144'/0'/0'");
    EXPECT_EQ(xpub->derive("")->toBase58(), "rM3cztMNEBwPZ7rAKzuKdw4LoVKBwVjxka");
}

TEST(RippleAddress, AddressFromPubKeyAndChainCodeAddressLevel) {
    std::vector<uint8_t> pubKey = hex::toByteArray(
            "03c5ee21cd4a30b5b8436a6b17047aceac5ed4e69ab5f96bde2af897ff80cefab1");
    std::vector<uint8_t> chainCode = hex::toByteArray(
            "0f2f1ffcfb379ceaef995d176e362e5b33ec14ba86d50f0facf46c2308f86c98");
    auto xpub = ledger::core::RippleLikeExtendedPublicKey::fromRaw(currency(),
                                                                   optional<std::vector<uint8_t >>(),
                                                                   pubKey,
                                                                   chainCode,
                                                                   "44'/144'/0'/0/0");
    EXPECT_EQ(xpub->derive("")->toBase58(), "rMspb4Kxa3EwdF4uN5TMqhHfsAkBit6w7k");
}

TEST(RippleAddress, XpubFromBase58String) {
    auto addr = "xpub6DECL9NX5hvkRrq98MRvXCjTP8s84NZUFReEVLizQMVH8epXyoMvncB9DG4d8kY94XPVYqFWtUVaaagZkXvje4AUF3qdd71fJc8KyhRRC8V";
    auto xpub = ledger::core::RippleLikeExtendedPublicKey::fromBase58(currency(), addr, optional<std::string>("44'/144'/0'"));
    EXPECT_EQ(xpub->toBase58(), addr);
    EXPECT_EQ(xpub->derive("0/0")->toBase58(), "rhnLXhSgTgiPNMuxjkjL4qqR3iajiMxdmQ");
    EXPECT_EQ(xpub->derive("0/1")->toBase58(), "rgnbQkpdM6SR4vjHmxRzdSP7kUoNsgEmy");
    EXPECT_EQ(xpub->derive("0/2")->toBase58(), "rn6kdEQif5eo2DEikofRTXCtUJRoRGJYFN");
    EXPECT_EQ(xpub->derive("0/3")->toBase58(), "r3mAmnNeJBdEyWHSK8uDbNmJc1f5dMyJKx");
}


TEST(RippleAddress, ParseAddressFromString) {
    auto address = "rwANfV4cFbvup4er3mhErm8h7i8mFG9PQF";
    auto rippleAddress = ledger::core::RippleLikeAddress::parse(address, currency(), Option<std::string>("0/0"));
    EXPECT_EQ(address, rippleAddress->toString());
}
