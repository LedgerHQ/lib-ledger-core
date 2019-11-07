/*
 *
 * fibonacci_test.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/10/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include <core/math/Fibonacci.hpp>

TEST(Fibonacci, Suite) {
    EXPECT_EQ(ledger::core::Fibonacci::compute(0), 0);
    EXPECT_EQ(ledger::core::Fibonacci::compute(1), 1);
    EXPECT_EQ(ledger::core::Fibonacci::compute(2), 1);
    EXPECT_EQ(ledger::core::Fibonacci::compute(3), 2);
    EXPECT_EQ(ledger::core::Fibonacci::compute(4), 3);
    EXPECT_EQ(ledger::core::Fibonacci::compute(5), 5);
    EXPECT_EQ(ledger::core::Fibonacci::compute(6), 8);
    EXPECT_EQ(ledger::core::Fibonacci::compute(7), 13);
    EXPECT_EQ(ledger::core::Fibonacci::compute(8), 21);
    EXPECT_EQ(ledger::core::Fibonacci::compute(9), 34);
    EXPECT_EQ(ledger::core::Fibonacci::compute(10), 55);
}
