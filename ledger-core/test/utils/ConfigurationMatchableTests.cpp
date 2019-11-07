/*
 *
 * configuration_matchable_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/05/2017.
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

#include <core/utils/ConfigurationMatchable.hpp>

using namespace ledger::core;

TEST(ConfigurationMatchable, MatchSameConfWithMultipleKeys) {
    auto conf = api::DynamicObject::newInstance()
            ->putString("string", "hey")
            ->putInt("int", 12)
            ->putLong("long", 12L << 32)
            ->putBoolean("bool", false)
            ->putData("data", {0x42, 0x42 >> 1, 0x42 >> 2});
    ConfigurationMatchable matchable({"string", "int", "long", "bool", "data"});
    matchable.setConfiguration(conf);
    EXPECT_TRUE(matchable.match(conf));
}

TEST(ConfigurationMatchable, MatchDifferentConfWithMultipleKeys) {
    auto conf = api::DynamicObject::newInstance()
            ->putString("string", "hey")
            ->putInt("int", 12)
            ->putLong("long", 12L << 32)
            ->putBoolean("bool", false)
            ->putData("data", {0x42, 0x42 >> 1, 0x42 >> 2});
    auto conf2 = api::DynamicObject::newInstance()
            ->putString("string", "ho")
            ->putLong("long", 13L << 32)
            ->putBoolean("bool", false)
            ->putData("data", {0x42, 0x42 >> 1, 0x42 >> 2});
    ConfigurationMatchable matchable({"string", "int", "long", "bool", "data"});
    matchable.setConfiguration(conf);
    EXPECT_FALSE(matchable.match(conf2));
}
