/*
 *
 * byteswriter_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 22/09/2016.
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

#include <ledger/core/bytes/BytesWriter.h>
#include <gtest/gtest.h>

using namespace ledger::core;

TEST(BytesWriter, WriteBeValues) {
    EXPECT_EQ(BytesWriter().writeBeValue(12U).toByteArray(), std::vector<uint8_t>({0x00, 0x00, 0x00, 12}));
    EXPECT_EQ(BytesWriter().writeBeValue((uint8_t)12).toByteArray(), std::vector<uint8_t>({12}));
    EXPECT_EQ(BytesWriter().writeBeValue(12ULL).toByteArray(), std::vector<uint8_t>({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 12}));
}

TEST(BytesWriter, WriteLeValues) {
    EXPECT_EQ(BytesWriter().writeLeValue(12U).toByteArray(), std::vector<uint8_t>({12, 0x00, 0x00, 00}));
    EXPECT_EQ(BytesWriter().writeLeValue((uint8_t)12).toByteArray(), std::vector<uint8_t>({12}));
    EXPECT_EQ(BytesWriter().writeLeValue(12ULL).toByteArray(), std::vector<uint8_t>({12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
}

TEST(BytesWriter, WriteVarInt) {
    EXPECT_EQ(BytesWriter().writeVarInt(0xA0).toByteArray(), std::vector<uint8_t>({0xA0}));
    EXPECT_EQ(BytesWriter().writeVarInt(0xA0B0).toByteArray(), std::vector<uint8_t>({0xFD, 0xB0, 0xA0}));
    EXPECT_EQ(BytesWriter().writeVarInt(0xA0B0C0D0).toByteArray(), std::vector<uint8_t>({0xFE, 0xD0, 0xC0, 0xB0, 0xA0}));
    EXPECT_EQ(BytesWriter().writeVarInt(0xA0B0C0D0E0F01011).toByteArray(), std::vector<uint8_t>({0xFF, 0x11, 0x10, 0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0}));
}
