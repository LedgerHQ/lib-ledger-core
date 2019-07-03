/*
 *
 * json_test.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/06/2019.
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
#include <utils/JsonParserPath.hpp>

using namespace ledger::core;

TEST(JsonParserPath, ToStringOnlyObjects) {
    JsonParserPath path;

    // { "foo" : { "bar": 12, "topaz": 13, "malachite": { "rare": 3 } } }
    path.startObject();
    path.key("foo");
    path.startObject();
    path.key("bar");
    path.value();
    EXPECT_EQ(path.toString(), "/foo/bar");
    path.key("topaz");
    path.value();
    EXPECT_EQ(path.toString(), "/foo/topaz");
    path.key("malachite");
    path.startObject();
    path.key("rare");
    path.value();
    EXPECT_EQ(path.toString(), "/foo/malachite/rare");
    path.endObject();
    EXPECT_EQ(path.toString(), "/foo/malachite");
    path.endObject();
    EXPECT_EQ(path.toString(), "/foo");
    path.endObject();

}

TEST(JsonParserPath, ToStringObjectsAndArray) {
    JsonParserPath path;

    // { "foo" : { "bar": 12, "topaz": [ {"rare" : 4}, {"useful" : 0 } ], "malachite": [ 13 ] } }
    path.startObject();
    path.key("foo");
    path.startObject();
    path.key("bar");
    path.value();
    EXPECT_EQ(path.toString(), "/foo/bar");
    path.key("topaz");
    path.startArray();
    path.startObject();
    path.key("rare");
    path.value();
    EXPECT_EQ(path.toString(), "/foo/topaz[0]/rare");
    path.endObject();
    path.startObject();
    path.key("useful");
    path.value();
    EXPECT_EQ(path.toString(), "/foo/topaz[1]/useful");
    path.endObject();
    EXPECT_EQ(path.toString(), "/foo/topaz[1]");
    path.endArray();
    EXPECT_EQ(path.toString(), "/foo/topaz");

    path.key("malachite");
    path.startArray();
    path.value();
    EXPECT_EQ(path.toString(), "/foo/malachite[0]");
    path.endArray();
    EXPECT_EQ(path.toString(), "/foo/malachite");
    path.endObject();
    EXPECT_EQ(path.toString(), "/foo");
    path.endObject();

}

TEST(JsonParserPath, MatchPaths) {
    JsonParserPathMatcher m_1("/foo/bar");
    JsonParserPathMatcher m_2("/foo/topaz[*]/rare");
    JsonParserPathMatcher m_2_bis("/foo/topaz?");
    JsonParserPathMatcher m_3("/foo/topaz[1]/useful");
    JsonParserPathMatcher m_3_wrong("/foo/topas[1]/useful");
    JsonParserPathMatcher m_4("/foo/topaz[1]");
    JsonParserPathMatcher m_5("/foo/*");
    JsonParserPathMatcher m_6("/*/malachite[*]");
    JsonParserPathMatcher m_7("/foo/malachite");
    JsonParserPathMatcher m_8("/foo");



    JsonParserPath path;

    // { "foo" : { "bar": 12, "topaz": [ {"rare" : 4}, {"useful" : 0 } ], "malachite": [ 13 ] } }
    path.startObject();
    path.key("foo");
    path.startObject();
    path.key("bar");
    path.value();
    EXPECT_TRUE(path.root().match(m_1));
    path.key("topaz");
    path.startArray();
    path.startObject();
    path.key("rare");
    path.value();
    EXPECT_TRUE(path.root().match(m_2));
    EXPECT_TRUE(path.root().match(m_2_bis));
    path.endObject();
    path.startObject();
    path.key("useful");
    path.value();
    EXPECT_TRUE(path.root().match(m_3));
    EXPECT_FALSE(path.root().match(m_3_wrong));
    path.endObject();
    EXPECT_TRUE(path.root().match(m_4));
    path.endArray();
    EXPECT_TRUE(path.root().match(m_5));
    path.key("malachite");
    path.startArray();
    path.value();
    EXPECT_TRUE(path.root().match(m_6));
    path.endArray();
    EXPECT_TRUE(path.root().match(m_7));
    path.endObject();
    EXPECT_TRUE(path.root().match(m_8));
    path.endObject();

}