/*
 *
 * bytesreader_tests
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

#include <gtest/gtest.h>

#include <core/bytes/BytesReader.hpp>

TEST(BytesReader, CursorTests) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 42, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0x12, 0x16});
    ledger::core::BytesReader reader(data);
    reader.read(3);
    EXPECT_EQ(reader.getCursor(), 3);
    EXPECT_EQ(reader.hasNext(), true);
    EXPECT_EQ(reader.available(), 14);
    reader.seek(1, ledger::core::BytesReader::Seek::CUR);
    EXPECT_EQ(reader.read(11), std::vector<uint8_t>({'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'}));
    EXPECT_EQ(reader.getCursor(), 15);
    EXPECT_EQ(reader.hasNext(), true);
    EXPECT_EQ(reader.available(), 2);
    reader.seek(2, ledger::core::BytesReader::Seek::CUR);
    EXPECT_EQ(reader.hasNext(), false);
    EXPECT_EQ(reader.available(), 0);
}

TEST(BytesReader, SubReaderCursorTests) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 42, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0x12, 0x16});
    ledger::core::BytesReader reader(data, 4, 11);
    EXPECT_EQ(reader.read(11), std::vector<uint8_t>({'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'}));
    EXPECT_EQ(reader.getCursor(), 11);
    EXPECT_EQ(reader.hasNext(), false);
    EXPECT_EQ(reader.available(), 0);
}

TEST(BytesReader, ReadString) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 42, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0x12, 0x16});
    ledger::core::BytesReader reader(data, 4, 11);
    EXPECT_EQ(reader.readString(11), "Hello world");
}

TEST(BytesReader, ReadNextString) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 42, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0});
    ledger::core::BytesReader reader(data, 4, 12);
    EXPECT_EQ(reader.readNextString(), "Hello world");
    EXPECT_EQ(reader.hasNext(), false);
}

TEST(BytesReader, ReadNextBeUint) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 42, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0});
    ledger::core::BytesReader reader(data);
    EXPECT_EQ(reader.readNextBeUint(), 0xFF01102A);
}

TEST(BytesReader, ReadNextLeUint) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 42, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0});
    ledger::core::BytesReader reader(data);
    EXPECT_EQ(reader.readNextLeUint(), 0x2A1001FF);
}

TEST(BytesReader, ReadNextBeUlong) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0});
    ledger::core::BytesReader reader(data);
    EXPECT_EQ(reader.readNextBeUlong(), 0xFF01100A0B0C0D0EUL);
}

TEST(BytesReader, ReadNextLeUlong) {
    std::vector<uint8_t> data({0xFF, 0x01, 0x10, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0});
    ledger::core::BytesReader reader(data);
    EXPECT_EQ(reader.readNextLeUlong(), 0x0E0D0C0B0A1001FFUL);
}

TEST(BytesReader, ReadNextVarInt) {
    EXPECT_EQ(ledger::core::BytesReader({0x0A}).readNextVarInt(), 0x0A);
    EXPECT_EQ(ledger::core::BytesReader({0xFD, 0x0B, 0x0A}).readNextVarInt(), 0x0A0B);
    EXPECT_EQ(ledger::core::BytesReader({0xFE, 0x0D, 0x0C, 0x0B, 0x0A}).readNextVarInt(), 0x0A0B0C0D);
    EXPECT_EQ(ledger::core::BytesReader({0xFF, 0x12, 0x11, 0x10, 0x0F, 0x0D, 0x0C, 0x0B, 0x0A}).readNextVarInt(), 0x0A0B0C0D0F101112);
}

TEST(BytesReader, ReadNextVarString) {
    std::vector<uint8_t> data({11, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0});
    ledger::core::BytesReader reader(data);
    EXPECT_EQ(reader.readNextVarString(), "Hello world");
}
