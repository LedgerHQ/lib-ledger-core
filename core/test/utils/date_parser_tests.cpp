/*
 *
 * date_parser_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/04/2017.
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

#include "utils/DateUtils.hpp"

#include "gtest/gtest.h"

using namespace ledger::core;

TEST(DateParser, ParseJSONDate) {
    auto date = "2012-04-23T18:25:43.511Z";
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::microseconds>(DateUtils::fromJSON(date).time_since_epoch()).count(), 1335205543000000);
}

TEST(DateParser, ParseJSONDate2) {
    auto date = "2017-03-27T09:10:22Z";
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::microseconds>(DateUtils::fromJSON(date).time_since_epoch()).count(), 1490605822000000);
}

TEST(DateParser, ToJSONDate) {
    auto date = "2017-03-27T09:10:22Z";
    auto tp   = DateUtils::fromJSON(date);
    EXPECT_EQ(DateUtils::toJSON(tp), date);
}

TEST(DateParser, FormatDate) {
    auto date = "2019-Jan-07 22:41:40";
    auto tp   = DateUtils::formatDateFromJSON(date);
    EXPECT_EQ(tp, "2019-01-7T22:41:40Z");
}