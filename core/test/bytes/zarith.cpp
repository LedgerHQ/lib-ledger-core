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
#include <ledger/core/bytes/BytesWriter.h>
#include <ledger/core/bytes/zarith/zarith.h>
#include <ledger/core/utils/hex.h>
#include <iostream>
using namespace std;

using namespace ledger::core;

TEST(ZarithTests, ParseCases) {
    auto result = zarith::zSerialize(hex::toByteArray("04fa"));
    EXPECT_EQ(hex::toString(result), "fa09");
    result = zarith::zSerialize(hex::toByteArray("27d8"));
    EXPECT_EQ(hex::toString(resultBis), "d84f");
    result = zarith::zSerialize(hex::toByteArray("02540be400"));
    EXPECT_EQ(hex::toString(resultBisBis), "80c8afa025");
}
