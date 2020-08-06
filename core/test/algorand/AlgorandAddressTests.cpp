/*
 * AlgorandAddressTests
 *
 * Created by Hakim Aammar on 11/05/2020.
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
#include <api/AlgorandAddress.hpp>
#include <api/Address.hpp>

#include <wallet/currencies.hpp>
#include <utils/hex.h>

#include <gtest/gtest.h>

using namespace ledger::core;

TEST(AlgorandAddressTest, AddressToPubkey) {
    auto address = "RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZM";
    auto pubKey = api::AlgorandAddress::toPublicKey(address);
    EXPECT_EQ(hex::toByteArray("89afdb83e3b65d9ec966e5a126a0aa41b826df9571f5bc0897efbbad8e0402f7"), pubKey);
}

TEST(AlgorandAddressTest, PubkeyToAddress) {
    auto pubKey = hex::toByteArray("89afdb83e3b65d9ec966e5a126a0aa41b826df9571f5bc0897efbbad8e0402f7");
    auto address = api::AlgorandAddress::fromPublicKey(pubKey);
    EXPECT_EQ("RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZM", address) << "!!! This test will fail if SHA512-256 is not available !!!";
}

TEST(AlgorandAddressTest, AddressValidation) {
    auto currency = currencies::ALGORAND;
    auto address = "RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZM";
    EXPECT_TRUE(api::Address::isValid(address, currency));
    address = "RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL50000";
    EXPECT_FALSE(api::Address::isValid(address, currency));
    address = "RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZMRGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZM";
    EXPECT_FALSE(api::Address::isValid(address, currency));
    address = "RGX5XA7DWZOZ5SLG4WQSNIFK";
    EXPECT_FALSE(api::Address::isValid(address, currency));
}