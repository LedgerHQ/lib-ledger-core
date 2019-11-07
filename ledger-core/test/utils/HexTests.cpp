/*
 *
 * hex_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2016.
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

#include <core/utils/Hex.hpp>

using namespace ledger::core;

TEST(Hex, toByteArray) {
    EXPECT_EQ(hex::toByteArray("DEADBEEF"), std::vector<uint8_t >({0xDE, 0xAD, 0xBE, 0xEF}));
    EXPECT_EQ(hex::toByteArray("EADBEEF"), std::vector<uint8_t >({0xE, 0xAD, 0xBE, 0xEF}));
    EXPECT_EQ(hex::toByteArray(""), std::vector<uint8_t >({}));
    EXPECT_EQ(hex::toByteArray("deadbeef"), std::vector<uint8_t >({0xDE, 0xAD, 0xBE, 0xEF}));
    EXPECT_EQ(hex::toByteArray("eAdBeEf"), std::vector<uint8_t >({0xE, 0xAD, 0xBE, 0xEF}));
}

TEST(Hex, toString) {
    EXPECT_EQ(hex::toString({0xDE, 0xAD, 0xBE, 0xEF}), "deadbeef");
    EXPECT_EQ(hex::toString({0xDE, 0xAD, 0xBE, 0xEF}, true), "DEADBEEF");
}
