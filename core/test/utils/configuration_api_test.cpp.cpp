/*
 *
 * configuration_api_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/01/2017.
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
#include <ledger/core/api/Configuration.hpp>

TEST(Configuration, PutAndRetrieve) {
    auto conf = ledger::core::api::Configuration::newInstance();
    conf->putBoolean("is_true", true);
    conf->putString("my_string", "a supa string!");
    conf->putInt("my_int", 42);
    conf->putData("my_data", {0x01B, 0x02B, 0x42, 0x42});

    EXPECT_EQ(conf->getBoolean("is_true", false), true);
    EXPECT_EQ(conf->getString("my_string"), "a supa string!");
    EXPECT_EQ(conf->getInt("my_int", 0), 42);
    EXPECT_EQ(conf->getData("my_data"), std::vector<uint8_t>({0x01B, 0x02B, 0x42, 0x42}));
}