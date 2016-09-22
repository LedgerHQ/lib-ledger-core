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
#ifndef LEDGER_CORE_BYTESWRITER_H
#define LEDGER_CORE_BYTESWRITER_H

#include <cstdio>
#include <vector>
#include "../utils/endian.h"

namespace ledger {
    namespace core {
        /**
         * Helper class to write byte array.
         */
        class BytesWriter {

        public:
            BytesWriter(size_t size);
            BytesWriter() {};

            inline BytesWriter& writeByte(uint8_t byte)  {
                _bytes.push_back(byte);
                return *this;
            }

            template<typename T> BytesWriter& writeBeValue(const T value) {
                auto ptr = reinterpret_cast<const uint8_t *>(&value);
                if (!ledger::core::endianness::isSystemBigEndian()) {
                    auto i = sizeof(value);
                    do {
                        i -= 1;
                        writeByte(ptr[i]);
                    } while (i > 0);
                } else {
                    for (auto i = 0; i < sizeof(value); i++) {
                        writeByte(ptr[i]);
                    }
                }
                return *this;
            }
            template<typename T> BytesWriter& writeLeValue(const T value) {
                auto ptr = reinterpret_cast<const uint8_t *>(&value);
                if (ledger::core::endianness::isSystemBigEndian()) {
                    auto i = sizeof(value);
                    do {
                        i -= 1;
                        writeByte(ptr[i]);
                    } while (i > 0);
                } else {
                    for (auto i = 0; i < sizeof(value); i++) {
                        writeByte(ptr[i]);
                    }
                }
                return *this;
            }

            std::vector<uint8_t> toByteArray() const;

        private:
            std::vector<uint8_t> _bytes;
        };
    }
}


#endif //LEDGER_CORE_BYTESWRITER_H
