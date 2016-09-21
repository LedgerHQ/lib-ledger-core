/**
 *
 * bigint_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/09/2016.
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
#include "../../include/math/BigInt.h"

using namespace ledger::core;

TEST(BigInt, InitializeWithIntValue) {
    auto value = BigInt(12);
    EXPECT_EQ(value.toInt(), 12);
    EXPECT_EQ(BigInt(-12).toInt(), -12);
}

TEST(BigInt, InitializeWithLowValueString) {
    auto value = BigInt::from_dec("12");
    EXPECT_EQ(value->toInt(), 12);
    delete value;
}

TEST(BigInt, ConvertToHexString) {
    auto hex = "0102030405060708090a0b0c0d0f11223344556677889900aabbccddeeff";
    auto value = BigInt::from_hex(hex);
    EXPECT_EQ(value->toHexString(), hex);
    delete value;
}

TEST(BigInt, ConvertToDecString) {
    auto dec = "-1234567890123456789012345678901234567890123467890123457890";
    auto value = BigInt::from_dec(dec);
    EXPECT_EQ(value->toString(), dec);
    delete value;
}

TEST(BigInt, CompareLt) {
    EXPECT_TRUE(BigInt("12") < BigInt("13"));
    EXPECT_FALSE(BigInt("12") < BigInt("-13"));
    EXPECT_TRUE(BigInt("1000000000000000000000000000000000000000000000") <
                BigInt("10000000000000000000000000000000000000000000000"));
    EXPECT_FALSE(BigInt("10000000000000000000000000000000000000000000000") <
                BigInt("1000000000000000000000000000000000000000000000"));
    EXPECT_FALSE(BigInt("-12") < BigInt("-13"));
    EXPECT_TRUE(BigInt("-13") < BigInt("-12"));
}

TEST(BigInt, CompareLte) {
    EXPECT_TRUE(BigInt("12") <= BigInt("13"));
    EXPECT_FALSE(BigInt("12") <= BigInt("-13"));
    EXPECT_TRUE(BigInt("1000000000000000000000000000000000000000000000") <=
                BigInt("10000000000000000000000000000000000000000000000"));
    EXPECT_FALSE(BigInt("10000000000000000000000000000000000000000000000") <=
                 BigInt("1000000000000000000000000000000000000000000000"));
    EXPECT_FALSE(BigInt("-12") <= BigInt("-13"));
    EXPECT_TRUE(BigInt("-13") <= BigInt("-12"));
    EXPECT_TRUE(BigInt("1000000000000000000000000000000000000000000000") <=
                BigInt("1000000000000000000000000000000000000000000000"));
    EXPECT_FALSE(BigInt("1000000000000000000000000000000000000000000000") <=
                 BigInt("-1000000000000000000000000000000000000000000000"));
}

TEST(BigInt, CompareEqual) {
    EXPECT_TRUE(BigInt("12") == BigInt("12"));
    EXPECT_FALSE(BigInt("12") == BigInt("13"));
    EXPECT_FALSE(BigInt("12") == BigInt("-12"));
    EXPECT_TRUE(BigInt("10000000000000000000000000000000000000000000000000000000") ==
                BigInt("10000000000000000000000000000000000000000000000000000000"));
}

#define EXPECT_B_EQ(x, y) EXPECT_EQ((x).to_string(), y.to_string())

TEST(BigInt, Add) {
    EXPECT_B_EQ((BigInt("12") + BigInt(12)), BigInt("24"));
    EXPECT_B_EQ((BigInt("12") + BigInt(-12)), BigInt("0"));
    EXPECT_B_EQ((BigInt("-12") + BigInt(-12)), BigInt("-24"));
    EXPECT_B_EQ((BigInt("-12") + BigInt(12)), BigInt("0"));
    EXPECT_B_EQ((BigInt("10000000000000000000000000000000000000000000000000000000000000000") +
                 BigInt("10000000000000000000000000000000000000000000000000000000000000000")),
                 BigInt("20000000000000000000000000000000000000000000000000000000000000000"));
    EXPECT_B_EQ((BigInt("10000000000000000000000000000000000000000000000000000000000000000") +
                 BigInt("-10000000000000000000000000000000000000000000000000000000000000000")),
                 BigInt("0"));
    EXPECT_B_EQ((BigInt("-10000000000000000000000000000000000000000000000000000000000000000") +
                 BigInt("10000000000000000000000000000000000000000000000000000000000000000")),
                 BigInt("0"));
    EXPECT_B_EQ((BigInt("-10000000000000000000000000000000000000000000000000000000000000000") +
                 BigInt("-10000000000000000000000000000000000000000000000000000000000000000")),
                 BigInt("-20000000000000000000000000000000000000000000000000000000000000000"));
}

TEST(BigInt, Subtract) {
    EXPECT_EQ(BigInt(42) - BigInt(26), BigInt(16));
    EXPECT_EQ(BigInt(42) - BigInt(-26), BigInt(68));
    EXPECT_EQ(BigInt(-42) - BigInt(26), BigInt(-68));
}

TEST(BigInt, Multiply) {
    EXPECT_EQ(BigInt(12) * BigInt(10), BigInt(120));
    EXPECT_EQ(BigInt(12) * BigInt(-10), BigInt(-120));
    EXPECT_EQ(BigInt(-12) * BigInt(-10), BigInt(120));
}

TEST(BigInt, Divide) {
    EXPECT_B_EQ(BigInt(12) / BigInt(10), BigInt(1));
    EXPECT_B_EQ(BigInt(12) / BigInt(-10), BigInt(-1));
    EXPECT_B_EQ(BigInt(-12) / BigInt(-10), BigInt(1));
    EXPECT_B_EQ(BigInt(-12) / BigInt(10), BigInt(-1));
}

TEST(BigInt, Mod) {
    EXPECT_B_EQ(BigInt(12) % BigInt(10), BigInt(2));
    EXPECT_B_EQ(BigInt(12) % BigInt(-10), BigInt(2));
    EXPECT_B_EQ(BigInt(-12) % BigInt(-10), BigInt(-2));
    EXPECT_B_EQ(BigInt(-12) % BigInt(10), BigInt(-2));
}

TEST(BigInt, Increment) {
    EXPECT_EQ(++BigInt(1), BigInt(2));
    EXPECT_B_EQ(++BigInt(-1), BigInt(0));
}

TEST(BigInt, Decrement) {
    EXPECT_EQ(--BigInt(1), BigInt(0));
    EXPECT_B_EQ(--BigInt(-1), BigInt(-2));
}

TEST(BigInt, Power) {
    EXPECT_EQ(BigInt(2).pow(2), BigInt(4));
    EXPECT_EQ(BigInt(-2).pow(2), BigInt(4));
    EXPECT_EQ(BigInt(1000000).pow(0), BigInt(1));
    EXPECT_EQ(BigInt(-1000000).pow(0), BigInt(-1));
    EXPECT_EQ(BigInt(-2).pow(3), BigInt(-8));
}