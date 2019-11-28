/*
 *
 * derivation_path_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 16/12/2016.
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

#include <core/utils/DerivationPath.hpp>

using namespace ledger::core;

TEST(DerivationPath, ParseDerivationPath) {
    EXPECT_EQ(DerivationPath("m/44'/0'/0'/0/0").toString(), "44'/0'/0'/0/0");
}

TEST(DerivationPath, ParseDerivationHexPath) {
    EXPECT_EQ(DerivationPath("m/44'/0'/0'/0/0x42").toString(), "44'/0'/0'/0/66");
}

TEST(DerivationPath, ParseInvalidDerivationPath) {
   EXPECT_THROW({
                    DerivationPath("m/this/is/invalid");
                }, Exception
   );
}

TEST(DerivationPath, ParseRoot) {
    EXPECT_TRUE(DerivationPath("m").isRoot());
}
