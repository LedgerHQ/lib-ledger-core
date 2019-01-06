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
#include <ledger/core/utils/hex.h>
#include <ledger/core/api/RippleLikeExtendedPublicKey.hpp>
#include <ledger/core/utils/optional.hpp>
#include <ledger/core/api/Networks.hpp>
#include <wallet/currencies.hpp>
#include <api/Address.hpp>
#include <ripple/RippleLikeExtendedPublicKey.h>
#include <utils/hex.h>

using namespace ledger::core::api;
using namespace ledger::core;

const Currency currency = currencies::RIPPLE;

TEST(Address, AddressFromPubKeyAndChainCodeAccountLevel) {
    std::vector<uint8_t> pubKey = hex::toByteArray("039ea82278f0057b8b041a146f810aa2d84b8ffdfaef3e625624706ad13e12b717");
    std::vector<uint8_t> chainCode = hex::toByteArray("abcc4933bec06eeca6628b9e44f8e71d5e3cf510c0450dd1e29d9aa0f1717da9");
    auto xpub = ledger::core::RippleLikeExtendedPublicKey::fromRaw(currency,
                                                                   optional<std::vector<uint8_t >>(),
                                                                   pubKey,
                                                                   chainCode,
                                                                   "44'/144'/0'/0'");
    EXPECT_EQ(xpub->derive("")->toBase58(), "rM3cztMNEBwPZ7rAKzuKdw4LoVKBwVjxka");
}

TEST(Address, AddressFromPubKeyAndChainCodeAddressLevel) {
    std::vector<uint8_t> pubKey = hex::toByteArray("03c5ee21cd4a30b5b8436a6b17047aceac5ed4e69ab5f96bde2af897ff80cefab1");
    std::vector<uint8_t> chainCode = hex::toByteArray("0f2f1ffcfb379ceaef995d176e362e5b33ec14ba86d50f0facf46c2308f86c98");
    auto xpub = ledger::core::RippleLikeExtendedPublicKey::fromRaw(currency,
                                                                   optional<std::vector<uint8_t >>(),
                                                                   pubKey,
                                                                   chainCode,
                                                                   "44'/144'/0'/0/0");
    EXPECT_EQ(xpub->derive("")->toBase58(), "rMspb4Kxa3EwdF4uN5TMqhHfsAkBit6w7k");
}




