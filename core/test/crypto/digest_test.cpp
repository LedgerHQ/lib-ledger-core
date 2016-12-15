/*
 *
 * digest_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/12/2016.
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
#include <ledger/core/crypto/SHA256.hpp>
#include <ledger/core/crypto/RIPEMD160.hpp>
#include <ledger/core/utils/hex.h>

using namespace ledger::core;

TEST(Digests, SHA256_Strings_to_String) {
    EXPECT_EQ(SHA256::stringToHexHash("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"), "2ff100b36c386c65a1afc462ad53e25479bec9498ed00aa5a04de584bc25301b");
}

TEST(Digests, SHA256_bytes_to_String) {
    EXPECT_EQ(SHA256::bytesToHexHash({0x2f, 0xf1, 0x00, 0xb3, 0x6c, 0x38, 0x6c, 0x65, 0xa1, 0xaf, 0xc4, 0x62, 0xad, 0x53,
                                      0xe2, 0x54, 0x79, 0xbe, 0xc9, 0x49, 0x8e, 0xd0, 0x0a, 0xa5, 0xa0, 0x4d, 0xe5, 0x84,
                                      0xbc, 0x25, 0x30, 0x1b}),
              "c21efda7db95c7c642cee4095df44b19a23cc43f72ee5cae87cbd0f32230d2ee");
}

TEST(Digests, RIPEMD160) {
    std::vector<std::vector<std::string>> fixtures = {
            {"", "9c1185a5c5e9fc54612808977ee8f548b2258d31"},
            {"a", "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe"},
            {"abc", "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc"},
            {"message digest", 	"5d0689ef49d2fae572b881b123a85ffa21595f36"}
    };
    for (auto& i : fixtures) {
        EXPECT_EQ(RIPEMD160::hash(std::vector<uint8_t>(i[0].begin(), i[0].end())), hex::toByteArray(i[1]));
    }
}