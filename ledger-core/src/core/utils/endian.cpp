/*
 *
 * endian
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/09/2016.
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

#include <cstdint>
#include <iostream>
#include <core/utils/Endian.hpp>

namespace ledger {
    namespace core {
        namespace endianness {
            Endianness getSystemEndianness() {
                short int number = 0x1;
                char *numPtr = (char *) &number;
                return ((numPtr[0] == 1) ? Endianness::LITTLE : Endianness::BIG);
            }

            bool isSystemBigEndian() {
                return getSystemEndianness() == Endianness::BIG;
            }

            bool isSystemLittleEndian() {
                return getSystemEndianness() == Endianness::LITTLE;
            }

            static void reverseBytes(uint8_t *ptr, size_t size) {
                unsigned char tmp;
                for (auto i = 0; i < (size / 2); i++) {
                    tmp = ptr[i];
                    ptr[i] = ptr[size - i - 1];
                    ptr[size - i - 1] = tmp;
                }
            }

            void *swapToBigEndian(void *ptr, size_t size) {
                if (getSystemEndianness() == Endianness::LITTLE) {
                    reverseBytes(static_cast<uint8_t *>(ptr), size);
                }
                return ptr;
            }

            void *swapToLittleEndian(void *ptr, size_t size) {
                if (getSystemEndianness() == Endianness::BIG) {
                    reverseBytes(static_cast<uint8_t *>(ptr), size);
                }
                return ptr;
            }

            void *swapToEndianness(void *ptr, size_t size, Endianness current, Endianness final) {
                if (current != final) {
                    reverseBytes(static_cast<uint8_t *>(ptr), size);
                }
                return ptr;
            }

            const void * int_to_array(int i, Endianness endianness) {
                uint8_t *data = new uint8_t[sizeof(i)];
                auto ptr = (const uint8_t *)(&i);
                for (auto index = 0; index < sizeof(i); index++) {
                    data[index] = ptr[index];
                }
                swapToEndianness(data, sizeof(i), getSystemEndianness(), endianness);
                return (const void *)data;
            }
        }
    }
}
