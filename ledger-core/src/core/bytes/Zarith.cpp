/*
 *
 * zarith
 *
 * Created by El Khalil Bellakrid on 08/05/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#include <core/bytes/Zarith.hpp>

namespace ledger {
    namespace core {
        namespace zarith {

            std::vector<uint8_t> zSerializeNumber(const std::vector<uint8_t> &inputData) {
                std::vector<uint8_t> result;
                size_t id = inputData.size() - 1;
                uint32_t offset = 0;
                bool needAdditionalByte = false;

                while (id >= 0 && id < inputData.size()) {
                    auto byte = inputData[id];

                    // We shift current byte
                    byte <<= offset;

                    // Take shifted bits from previous byte
                    byte |= id < inputData.size() - 1 ? inputData[id + 1] >> (7 - (offset - 1)) : 0x00;

                    // This boolean will let us know if shifting last byte of inputData
                    // will generate a new byte (with remaining shifted bytes)
                    bool isFirst = id == 0;
                    needAdditionalByte = isFirst && inputData[id] >> (7 - offset);

                    // We set first bit to 1 to know that there is a coming byte
                    byte |= 0x80;

                    // Push to result modified byte
                    result.push_back(byte);

                    // Update offset
                    offset = (offset + 1) % 8;

                    // Update index if not a period or first iteration
                    if (offset != 0) {
                        id --;
                    }
                }

                if (needAdditionalByte) {
                    if (offset != 0) {
                        id ++;
                    }
                    offset = (offset - 1) % 8;
                    result.push_back(inputData[id] >> (7 - offset));
                } else {
                    result[result.size() - 1] &= 0x7F;
                }

                return result;
            }

            std::vector<uint8_t> zParse(const std::vector<uint8_t> &inputData) {
                // Reverse because result corresponds to Little Endian
                BytesReader bytesReader(inputData);

                return zParse(bytesReader);
            }

            std::vector<uint8_t> zParse(BytesReader &reader) {
                // We get bytes to parse without altering
                // bytes reader's state
                auto cursor = reader.getCursor();
                std::vector<uint8_t> data(reader.readUntilEnd());
                reader.reset();
                reader.read(cursor);

                std::vector<uint8_t> result;
                size_t id = 0, delay = 0;
                uint8_t offset = 0;
                while (id + delay < data.size()) {
                    auto byte = data[id + delay];
                    auto isLast = id + delay == data.size() - 1;

                    // We should stop reading if most significant bit is null
                    auto shouldStop = !(byte & 0x80);

                    // Mask last bit, which is here just to let us
                    // know if there is more data to read
                    byte &= data[id + delay] & 0x7F;

                    // Shift to retrieve only bits of original byte
                    byte >>= offset;

                    // Get shifted bits that has been moved to next byte
                    byte |= !isLast && !shouldStop ? data[id + delay + 1] << (7 - offset) : 0x00;

                    if (!(isLast && byte == 0)) { // This condition is here to prevent having  unnecessary leading null byte
                        result.push_back(byte);
                    }

                    // Update
                    id++;
                    delay = id / 7;
                    offset++;
                    offset %= 7;

                    if (shouldStop) {
                        break;
                    }
                }

                // Update bytes reader
                auto min = std::min<unsigned long>(id + delay, data.size());
                reader.read(min);

                // Little endianess
                std::reverse(result.begin(), result.end());

                return result;
            }        
        }
    }    
}