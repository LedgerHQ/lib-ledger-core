/*
 *
 * script_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 09/04/2018.
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
#include <wallet/bitcoin/scripts/BitcoinLikeScript.h>
#include <utils/hex.h>

using namespace ledger::core;
using namespace btccore;

TEST(Script, CreateSimpleScript) {
    BitcoinLikeScript script;
    script << OP_DUP << std::vector<uint8_t>({0x01, 0x02, 0x03});
    EXPECT_EQ(script.toString(), "OP_DUP PUSHDATA(3)[010203]");
}

TEST(Script, ParseP2PKHScript) {
    auto script = BitcoinLikeScript::parse(hex::toByteArray("76a9148829b0621743cde58974064e1e872d67eb4ea0c588ac"));
    EXPECT_TRUE(script.isSuccess());
    EXPECT_EQ(script.getValue().toString(), "OP_DUP OP_HASH160 PUSHDATA(20)[8829b0621743cde58974064e1e872d67eb4ea0c5] OP_EQUALVERIFY OP_CHECKSIG");
}

TEST(Script, ParseP2WPKHScript) {
    auto script = BitcoinLikeScript::parse(hex::toByteArray("00141d0f172a0ecb48aee1be1f2687d2963ae33f71a1"));
    EXPECT_TRUE(script.isSuccess());
    EXPECT_EQ(script.getValue().toString(), "OP_0 PUSHDATA(20)[1d0f172a0ecb48aee1be1f2687d2963ae33f71a1]");
}

TEST(Script, ParseP2WSHScript) {
    auto script = BitcoinLikeScript::parse(hex::toByteArray("00205d1b56b63d714eebe542309525f484b7e9d6f686b3781b6f61ef925d66d6f6a0"));
    EXPECT_TRUE(script.isSuccess());
    EXPECT_EQ(script.getValue().toString(), "OP_0 PUSHDATA(32)[5d1b56b63d714eebe542309525f484b7e9d6f686b3781b6f61ef925d66d6f6a0]");
}