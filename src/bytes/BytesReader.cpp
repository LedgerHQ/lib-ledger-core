/*
 *
 * BytesReader
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/09/2016.
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

#include "../../include/bytes/BytesReader.h"

namespace ledger {

    namespace core {

        std::vector<uint8_t> BytesReader::read(size_t length) {
            auto bytes = std::vector<uint8_t>(length);
            for (auto index = 0; index < length; index++) {
                bytes[index] = readNextByte();
            }
            return bytes;
        }

        void BytesReader::seek(int length) {
            _offset += length;
        }

        std::string BytesReader::readString(size_t length) {
            return std::string();
        }

        BigInt BytesReader::readLeBigInteger(size_t length) {
            return BigInt(0);
        }

        BigInt BytesReader::readBeBigInteger(size_t length) {
            return BigInt(0);
        }

        uint8_t BytesReader::readNextByte() {
            return 0;
        }

        unsigned int BytesReader::readNextBeUnsignedInt() {
            return 0;
        }

        unsigned int BytesReader::readNextLeUnsignedInt() {
            return 0;
        }

        int BytesReader::readNextBeInt() {
            return 0;
        }

        int BytesReader::readNextLeInt() {
            return 0;
        }

    }
}