/*
 *
 * bigint_api_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 04/11/2016.
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

#include <core/api/BigInt.hpp>

using namespace ledger::core::api;

#define DEC(x) BigInt::fromIntegerString((x), 10)

TEST(BigIntApi, InitializeWithString) {
    EXPECT_EQ("1000", BigInt::fromIntegerString("1000", 10)->toString(10));
}

TEST(BigIntApi, InitializeWithHexString) {
    EXPECT_EQ("10a851", BigInt::fromIntegerString("10a851", 16)->toString(16));
}

TEST(BigIntApi, AddTwoBigInt) {
    EXPECT_EQ("1337", DEC("1030")->add(DEC("307"))->toString(10));
}

TEST(BigIntApi, SubtractTwoBigInt) {
    EXPECT_EQ("1337", DEC("10000")->subtract(DEC("8663"))->toString(10));
}

TEST(BigIntApi, Pow) {
    EXPECT_EQ("4", DEC("2")->pow(2)->toString(10));
}

TEST(BigIntApi, MultiplyTwoBigInt) {
    EXPECT_EQ("1337", DEC("191")->multiply(DEC("7"))->toString(10));
}

TEST(BigIntApi, DivideTwoBigInt) {
    EXPECT_EQ("1337", DEC("9359")->divide(DEC("7"))->toString(10));
}

TEST(BigIntApi, FromDecimalString) {
    EXPECT_EQ("133713370000000000000000000000000000",
            BigInt::fromDecimalString("1 337.1337", 32, ".")->toString(10));
}
