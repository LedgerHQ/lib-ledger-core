/*
 *
 * BytesWriter
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

#include <algorithm>

#include <core/bytes/BytesWriter.h>
#include <core/utils/endian.h>

namespace ledger {
    namespace core {
        BytesWriter::BytesWriter(size_t size) {
            _bytes = std::vector<uint8_t>(size);
        }

        std::vector<uint8_t> BytesWriter::toByteArray() const {
            return _bytes;
        }

        BytesWriter &BytesWriter::writeByteArray(const std::vector<uint8_t> &data) {
            for (auto byte : data) {
                writeByte(byte);
            }
            return *this;
        }

        BytesWriter &BytesWriter::writeLeByteArray(const std::vector<uint8_t> &data) {
            for (auto it = data.rbegin(); it != data.rend(); it++) {
                writeByte(*it);
            }
            return *this;
        }

        BytesWriter &BytesWriter::writeBeBigInt(const BigInt &i) {
            auto bytes = i.toByteArray();
            return writeByteArray(bytes);
        }

        BytesWriter &BytesWriter::writeLeBigInt(const BigInt &i) {
            auto bytes = i.toByteArray();
            std::reverse(bytes.begin(), bytes.end());
            return writeByteArray(bytes);
        }

        BytesWriter &BytesWriter::writeString(const std::string &str) {
            for (auto c : str) {
                writeByte(c);
            }
            return *this;
        }

        BytesWriter &BytesWriter::writeVarInt(uint64_t i) {
            if (i < 0xFD) {
                return writeByte((uint8_t)i);
            } else if (i <= 0xFFFF) {
                return writeByte(0xFD).writeLeValue((uint16_t)i);
            } else if (i <= 0xFFFFFFFF) {
                return writeByte(0xFE).writeLeValue((uint32_t)i);
            } else {
                return writeByte(0xFF).writeLeValue(i);
            }
        }

        BytesWriter &BytesWriter::writeVarString(const std::string &str) {
            return writeVarInt(str.length()).writeString(str);
        }
    }
}
