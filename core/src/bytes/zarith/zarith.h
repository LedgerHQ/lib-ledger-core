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


#ifndef LEDGER_CORE_ZARITH_H
#define LEDGER_CORE_ZARITH_H

#include <algorithm>
#include <vector>
#include <bytes/BytesReader.h>
#include <utils/hex.h>

namespace ledger {
    namespace core {
        class zarith {
        public:
            /*
             * Most significant bit of each byte is replaced with 1 to let
             * us know there is still something to read, else 0
             * To sum up, if the resulting bytes are "chained" together (little endian) and we drop
             * the most significant bit of each byte, we get the input ("chained") bytes
             * e.g.
             * Input:           ad d9                  10101101 11011001  (I)
             * Output:          d9 db 02      11011001 11011011 00000010
             * LE:              02 db d9      00000010 11011011 11011001
             * Drop 1st bit:            (0)0000010 (1)1011011 (1)1011001  == (I)
            */
            static std::vector<uint8_t> zSerialize(const std::vector<uint8_t> &inputData) {
                // Reverse because result corresponds to Little Endian
                std::vector<uint8_t> data{inputData};
                std::reverse(data.begin(), data.end());

                std::vector<uint8_t> result;
                size_t id = 0, delay = 0; // delay is used when offset > 7
                uint8_t offset = 0;
                while (id - delay < data.size()) {
                    auto byte = data[id - delay];

                    // We shift current byte
                    byte <<= offset;

                    // Take shifted bits from previous byte
                    byte |= id - delay > 0 ? data[id - delay - 1] >> (7 - (offset - 1)) : 0x00;

                    // This boolean will let us know if shifting last byte of inputData
                    // will generate a new byte (with remaining shifted bytes)
                    bool isLast = data[id - delay] == data.back();
                    bool needAdditionalByte = isLast && data[id - delay] >> (7 - offset);

                    // We set first bit to 1 to know that there is a coming byte
                    if (!isLast || needAdditionalByte) {
                        byte |= 0x80;
                    }

                    // Push to result modified byte
                    result.push_back(byte);
                    if (needAdditionalByte) {
                        result.push_back(data[id - delay] >> (7 - offset));
                    }

                    // Update offset: if offset % 7 == 0 then following byte
                    // is not shifted i.e. offset = 0
                    offset = offset > 0 && !(offset % 7) ? 0 : offset + 1;

                    // If we shifted with [id/7] > 7 then we have a "delay"
                    // meaning that we should read values from "delayed" byte
                    delay = id / 7;

                    id++;
                }
                return result;
            };

            static std::vector<uint8_t> zParse(const std::vector<uint8_t> &inputData, bool forceContinue = false) {
                // Reverse because result corresponds to Little Endian
                BytesReader bytesReader(inputData);
                return zParse(bytesReader);
            };

            // The mode where we force parser to continue parsing even when finding
            // a null bit was made just for testing purpose
            static  std::vector<uint8_t> zParse(BytesReader &reader, bool forceContinue = false) {
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
                    auto isLast = byte == data.back();

                    // We should stop reading if most significant bit is null
                    auto shouldStop = !forceContinue && !(byte & 0x80);

                    // Mask last bit, which is here just to let us
                    // know if there is more data to read
                    byte &= 0x7F;

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
                auto max = std::min<unsigned long>(id + delay, data.size());
                reader.read(max);

                // Little endianess
                std::reverse(result.begin(), result.end());

                return result;
            };
        };
    }
}
#endif //LEDGER_CORE_ZARITH_H
