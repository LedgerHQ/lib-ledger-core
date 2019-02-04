/*
 *
 * derivation_scheme_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/04/2017.
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

#include "gtest/gtest.h"
#include "utils/DerivationScheme.hpp"

using namespace ledger::core;

TEST(DerivationScheme, SimpleCases) {
    DerivationScheme scheme("16/<account>'/<coin_type>");
    EXPECT_EQ(scheme.setAccountIndex(2).setCoinType(1990).getPath().toString(), "16/2'/1990");
}

TEST(DerivationScheme, BIP44) {
    DerivationScheme scheme("44'/<coin_type>'/<account>'/<node>/<address>");
    scheme
      .setCoinType(0)
      .setAccountIndex(1)
      .setNode(1)
      .setAddressIndex(42);
    EXPECT_EQ(scheme.getPath().toString(), "44'/0'/1'/1/42");
}

TEST(DerivationScheme, PartialScheme) {
    DerivationScheme scheme("44'/<coin_type>'/<account>'/<node>/<address>");
    auto xpubScheme = scheme.getSchemeTo(DerivationSchemeLevel::ACCOUNT_INDEX);
    auto accountScheme = scheme.getSchemeFrom(DerivationSchemeLevel::NODE);
    EXPECT_EQ(xpubScheme.toString(), "44'/<coin_type>'/<account>'");
    EXPECT_EQ(accountScheme.toString(), "<node>/<address>");
}