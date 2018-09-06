/*
 *
 * either_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/02/2017.
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
#include <ledger/core/utils/Either.hpp>
#include <ledger/core/utils/Exception.hpp>
#include <string>

using  namespace ledger::core;

TEST(Either, TestLeft) {
    Either<std::string, int> result = 12;
    EXPECT_TRUE(result.isRight());
    EXPECT_EQ(result.getRight(), 12);
    EXPECT_THROW(result.getLeft(), std::exception);
}

TEST(Either, TestRight) {
    Either<std::string, int> result = std::string("I got an error");
    EXPECT_TRUE(result.isLeft());
    EXPECT_EQ(result.getLeft(), "I got an error");
    EXPECT_THROW(result.getRight(), std::exception);
}

TEST(Either, TestFold) {
    {
        Either<std::string, int> result = std::string("42");
        auto folded = result.fold<int>([](const std::string &s) {
            return std::atoi(s.c_str());
        }, [](const int &i) {
            return i;
        });
        EXPECT_EQ(folded, 42);
    }
    {
        Either<std::string, int> result = 42;
        auto folded = result.fold<int>([](const std::string &s) {
            return std::atoi(s.c_str());
        }, [](const int &i) {
            return i;
        });
        EXPECT_EQ(folded, 42);
    }
}