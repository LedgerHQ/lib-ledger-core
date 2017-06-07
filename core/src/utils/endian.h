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

#undef BIG_ENDIAN
#undef LITTLE_ENDIAN

#ifndef LEDGER_CORE_ENDIAN_H
#define LEDGER_CORE_ENDIAN_H

#include <cstddef>
#include <cstdlib>

/**
 * Ledger global namespace
 */
namespace ledger {

    namespace core {

        /**
         * Endianness management functions
         * @headerfile endian.h <ledger/core/utils/endian.h>
         */
        namespace endianness {

            enum struct Endianness {
                BIG = 1,
                LITTLE
            };

            /**
             * Checks if the current system uses big endian or little bytes order.
             * @headerfile endian.h <ledger/core/utils/endian.h>
             * @return
             */
            Endianness getSystemEndianness();

            /**
             * Returns true if the current runtime uses big endian bytes order.
             * @return
             */
            bool isSystemBigEndian();

            /**
             * Returns true if the current runtime uses little endian bytes order.
             * @return
             */
            bool isSystemLittleEndian();

            /**
             * Swaps ptr bytes to big endian bytes order if the current runtime uses little endia bytes order.
             * @param ptr The buffer to force in big endian bytes order
             * @param size The size of the buffer
             * @return
             */
            void *swapToBigEndian(void *ptr, size_t size);

            /**
            * Swaps ptr bytes to little endian bytes order if the current runtime uses big endia bytes order.
            * @param ptr The buffer to force in little endian bytes order
            * @param size The size of the buffer
            * @return
            */
            void *swapToLittleEndian(void *ptr, size_t size);

            /**
             * Swaps ptr bytes to final endianness only if necessary
             * @param ptr The buffer to force in "final" endianness
             * @param size The size of the buffer
             * @param current The current byte order of ptr
             * @param final The desired final byte order of ptr
             * @return
             */
            void *swapToEndianness(void *ptr, size_t size, Endianness current, Endianness final);

            const void* int_to_array(int i, Endianness endianness);
            const void* unsigned_long_long_to_array(unsigned long long i, Endianness endianness);

            template <typename T>
            void* scalar_type_to_array(T i, Endianness endianness) {
                uint8_t *data = (uint8_t *) std::malloc(sizeof(i));
                auto ptr = (const uint8_t *)(&i);
                for (auto index = 0; index < sizeof(i); index++) {
                    data[index] = ptr[index];
                }
                swapToEndianness(data, sizeof(i), getSystemEndianness(), endianness);
                return (void *)data;
            }
        }
    }
}

#endif //LEDGER_CORE_ENDIAN_H
