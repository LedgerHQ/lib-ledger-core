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

#include <bytes/BytesReader.h>
#include <vector>
namespace ledger {
    namespace core {
        class zarith {
        public:

            static std::vector<uint8_t> zSerialize(const std::vector<uint8_t> &inputData) {
                // Reverse because result corresponds to Little Endian
                std::vector<uint8_t> data{inputData};
                std::reverse(data.begin(), data.end());

                std::vector<uint8_t> result;
                auto id = 0, delay = 0; // delay is used when offset > 7
                while (id - delay < data.size()) {
                    auto byte = data[id - delay];
                    if (id - delay > 0 && id % 7) {
                        byte <<= id % 7;
                        byte &= 0x7F;
                        byte |= data[id - delay - 1] >> (7 - ((id - 1) % 7));
                    }
                    byte |= id - delay != data.size() - 1 ? 0x80 : 0x00;
                    result.push_back(byte);
                    id++;
                    delay = id / 7;
                }
                return result;
            };

//            std::vector<uint8_t> zParse(const std::vector<uint8_t> &inputData) {
//                // Reverse because result corresponds to Little Endian
//                std::vector<uint8_t> data{inputData};
//                std::reverse(data.begin(), data.end());
//                std::vector<uint8_t> result;
//                // TODO
//                return result;
//            };

            static  std::vector<uint8_t> zParse(BytesReader &reader) {
                // TODO
                std::vector<uint8_t> result;
                return result;
            };
        };
    }
}
#endif //LEDGER_CORE_ZARITH_H
