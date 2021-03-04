/*
 *
 * ttlvar_test
 * ledger-core
 *
 * Created by Rémi Barjon on 05/02/2021.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ledger
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
#include <ledger/core/utils/TTLVar.hpp>
#include <chrono>
#include <thread>

using namespace ledger::core;

TEST(TTLVar, GetAlive) {
    const TTLVar<int> ttlvar(4, std::chrono::seconds(5));
    const auto var = ttlvar.get();

    EXPECT_TRUE(var.hasValue());
    EXPECT_EQ(var.getValue(), 4);
}

TEST(TTLVar, GetNotAlive) {
    const TTLVar<int> ttlvar(4, std::chrono::seconds(0));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto var = ttlvar.get();

    EXPECT_TRUE(var.isEmpty());
}

TEST(TTLVar, Update) {
    const TTLVar<int> ttlvar(0, std::chrono::seconds(5));
    ttlvar.update(4);
    const auto var = ttlvar.get();

    EXPECT_TRUE(var.hasValue());
    EXPECT_EQ(var.getValue(), 4);
}
