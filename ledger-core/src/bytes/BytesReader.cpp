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

#include "BytesReader.h"
#include "../utils/endian.h"
#include <fmt/format.h>
#include <sstream>
#include <algorithm>


namespace ledger {

    namespace core {

        BytesReader::BytesReader(const std::vector<uint8_t> &data, unsigned long offset, unsigned long length) {
            _bytes = data;
            _offset = offset;
            _length = length;
            _cursor = offset;
        }

        void BytesReader::seek(long offset, BytesReader::Seek origin) {
            unsigned long off = 0;
            switch (origin) {
                case Seek::CUR :
                    off = _cursor + offset;
                    break;
                case Seek::SET :
                    off = _offset;
                    break;
                case Seek::END :
                    off = _offset + _length + offset;
                    break;
            }
            if (off < _offset) {
                throw std::out_of_range(fmt::format("Offset [{}] is too low (minimum value is {})", off, _offset));
            } else if (off > (_offset + _length)) {
                throw std::out_of_range(fmt::format("Offset [{}] is too high (maximum value is {})", off, _offset + _length));
            }
            _cursor = off;
        }

        std::vector<uint8_t> BytesReader::read(unsigned long length) {
           std::vector<uint8_t> out(length);
            read(length, out);
            return out;
        }
        void BytesReader::reset() {
            _cursor = 0;
            _offset = 0;
        }
        unsigned long BytesReader::getCursor() const {
            return _cursor - _offset;
        }

        bool BytesReader::hasNext() const {
            return available() != 0;
        }

        unsigned long BytesReader::available() const {
            return (_length + _offset) - _cursor;
        }

        std::string BytesReader::readString(unsigned long length) {
            std::string result(length, '0');
            auto index = 0UL;
            for (auto byte : read(length)) {
                result[index] = byte;
                index += 1;
            }
            return result;
        }

        uint8_t BytesReader::readNextByte() {
            uint8_t result = _bytes[_cursor];
            seek(1, Seek::CUR);
            return result;
        }

        std::string BytesReader::readNextString() {
            std::ostringstream oss;
            uint8_t byte;
            do {
                byte = readNextByte();
                if (byte != '\0')
                    oss << byte;
            } while (byte != '\0');
            return oss.str();
        }

        uint32_t BytesReader::readNextBeUint() {
            uint32_t result;
            uint8_t* ptr = reinterpret_cast<uint8_t *>(&result);
            for (auto i = 0; i < sizeof(result); i++) {
                ptr[i] = readNextByte();
            }
            ledger::core::endianness::swapToEndianness(ptr, sizeof(result),
                                                        ledger::core::endianness::Endianness::BIG,
                                                        ledger::core::endianness::getSystemEndianness());
            return result;
        }

        uint32_t BytesReader::readNextLeUint() {
            uint32_t result;
            uint8_t* ptr = reinterpret_cast<uint8_t *>(&result);
            for (auto i = 0; i < sizeof(result); i++) {
                ptr[i] = readNextByte();
            }
            ledger::core::endianness::swapToEndianness(ptr, sizeof(result),
                                                       ledger::core::endianness::Endianness::LITTLE,
                                                       ledger::core::endianness::getSystemEndianness());
            return result;
        }

        uint64_t BytesReader::readNextBeUlong() {
            uint64_t result;
            uint8_t* ptr = reinterpret_cast<uint8_t *>(&result);
            for (auto i = 0; i < sizeof(result); i++) {
                ptr[i] = readNextByte();
            }
            ledger::core::endianness::swapToEndianness(ptr, sizeof(result),
                                                       ledger::core::endianness::Endianness::BIG,
                                                       ledger::core::endianness::getSystemEndianness());
            return result;
        }

        uint64_t BytesReader::readNextLeUlong() {
            uint64_t result;
            uint8_t* ptr = reinterpret_cast<uint8_t *>(&result);
            for (auto i = 0; i < sizeof(result); i++) {
                ptr[i] = readNextByte();
            }
            ledger::core::endianness::swapToEndianness(ptr, sizeof(result),
                                                       ledger::core::endianness::Endianness::LITTLE,
                                                       ledger::core::endianness::getSystemEndianness());
            return result;
        }

        ledger::core::BigInt BytesReader::readNextBeBigInt(size_t bytes) {
            std::vector<uint8_t> data(bytes);
            read(bytes, data);
            return BigInt(data.data(), data.size(), false);
        }

        ledger::core::BigInt BytesReader::readNextLeBigInt(size_t bytes) {
            std::vector<uint8_t> data(bytes);
            read(bytes, data);
            std::reverse(data.begin(), data.end());
            return BigInt(data.data(), data.size(), false);
        }

        uint64_t BytesReader::readNextVarInt() {
            uint8_t size = readNextByte();
            switch (size) {
                case 0xFD:
                    size = 2;
                    break;
                case 0xFE:
                    size = 4;
                    break;
                case 0xFF:
                    size = 8;
                    break;
                default:
                    return size;
            }
            return readNextLeBigInt(size).toUint64();
        }

        std::string BytesReader::readNextVarString() {
            uint64_t length = readNextVarInt();
            return readString(length);
        }

        uint8_t BytesReader::peek() const {
            return _bytes[_cursor];
        }

        std::vector<uint8_t> BytesReader::readUntilEnd() {
            return read(available());
        }

        void BytesReader::read(unsigned long length, std::vector<uint8_t> &data) {
            for (auto i = 0; i < length; i++) {
                data[i] = _bytes[_cursor];
                seek(1, Seek::CUR);
            }
        }


    }
}
