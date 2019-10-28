/*
 *
 * zarith
 *
 * Created by El Khalil Bellakrid on 08/05/2019.
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
#include <iostream>
#include <vector>

#include <core/bytes/BytesWriter.hpp>
#include <core/bytes/zarith/Zarith.hpp>
#include <core/utils/Hex.hpp>

using namespace std;

using namespace ledger::core;

TEST(ZarithTests, ParseCases) {
    auto res = zarith::zSerializeNumber(hex::toByteArray("04fa"));
    EXPECT_EQ(hex::toString(res), "fa09");

    res = zarith::zParse(hex::toByteArray("fa09"));
    EXPECT_EQ(hex::toString(res), "04fa");

    res = zarith::zSerializeNumber(hex::toByteArray("27d8"));
    EXPECT_EQ(hex::toString(res), "d84f");

    res = zarith::zParse(hex::toByteArray("d84f"));
    EXPECT_EQ(hex::toString(res), "27d8");

    res = zarith::zSerializeNumber(hex::toByteArray("02540be400"));
    EXPECT_EQ(hex::toString(res), "80c8afa025");

    res = zarith::zParse(hex::toByteArray("80c8afa025"));
    EXPECT_EQ(hex::toString(res), "02540be400");

    res = zarith::zSerializeNumber(hex::toByteArray("2710"));
    EXPECT_EQ(hex::toString(res), "904e");

    res = zarith::zParse(hex::toByteArray("904e"));
    EXPECT_EQ(hex::toString(res), "2710");

    res = zarith::zSerializeNumber(hex::toByteArray("115"));
    EXPECT_EQ(hex::toString(res), "9502");

    res = zarith::zSerializeNumber(hex::toByteArray("3b9aca00"));
    EXPECT_EQ(hex::toString(res), "8094ebdc03");

    res = zarith::zParse(hex::toByteArray("8094ebdc03"));
    EXPECT_EQ(hex::toString(res), "3b9aca00");

    res = zarith::zSerializeNumber(hex::toByteArray("add9"));
    EXPECT_EQ(hex::toString(res), "d9db02");

    res = zarith::zParse(hex::toByteArray("d9db02"));
    EXPECT_EQ(hex::toString(res), "add9");

    res = zarith::zSerializeNumber(hex::toByteArray("b204d6f302540be400"));
    EXPECT_EQ(hex::toString(res), "80c8afa0a5e0bceb84e402");

    res = zarith::zParse(hex::toByteArray("80c8afa0a5e0bceb84e402"));
    EXPECT_EQ(hex::toString(res), "b204d6f302540be400");

    res = zarith::zParse(hex::toByteArray("d84f9502"));
    EXPECT_EQ(hex::toString(res), "27d8");

    res = zarith::zSerializeNumber(hex::toByteArray("ff00000000000001"));
    EXPECT_EQ(hex::toString(res), "8180808080808080ff01");

    res = zarith::zParse(hex::toByteArray("8180808080808080ff01"));
    EXPECT_EQ(hex::toString(res), "ff00000000000001");

    res = zarith::zSerializeNumber(hex::toByteArray("0100ff000000000000000000000001"));
    EXPECT_EQ(hex::toString(res), "81808080808080808080808080e0bf8001");

    res = zarith::zParse(hex::toByteArray("81808080808080808080808080e0bf8001"));
    EXPECT_EQ(hex::toString(res), "0100ff000000000000000000000001");
}
