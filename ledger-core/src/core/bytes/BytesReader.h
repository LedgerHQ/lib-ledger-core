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

#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <core/math/BigInt.h>
#include <core/ledger-core.h>

namespace ledger {
    namespace core {
        /**
         * A helper class used to symplify parsing of data. Beware that every reading or seeking method can throw
         * a std::out_of_range exception.
         */
        class BytesReader {

        public:
            enum Seek {
                SET,
                CUR,
                END
            };

        public:
            /**
             * Creates a new bytes reader starting at the given offset and able to read up to length bytes.
             * @param data The data to read.
             * @param offset The reader will start reading at this index in the vector.
             * @param length The maximum size on which the reader can read data.
             */
            BytesReader(const std::vector<uint8_t>& data, unsigned long offset, unsigned long length);
            /**
             * Creates a new bytes reader starting at byte 0 and able to read bytes until the end of data.
             * @param data The data to read.
             * @return
             */
            BytesReader(const std::vector<uint8_t>& data) : BytesReader(data, 0, data.size()) {};

            /**
             * Sets the position indicator associated with the BytesReader to a new position.
             * @param offset Number of bytes to offset from origin.
             * @param origin Position used as reference for the offset.
             */
            void seek(long offset, Seek origin);
            uint8_t peek() const;
            /**
             * Reads *length* bytes and advance the cursor in the reader.
             * @param length Number of bytes to read.
             * @return A vector of read bytes.
             */
            std::vector<uint8_t> read(unsigned long length);
            void read(unsigned long length, std::vector<uint8_t>& out);

            /**
             * Reads a single byte.
             * @return
             */
            uint8_t readNextByte();

            /**
             * Reads the next string with the given length.
             * @param length The length of the string.
             * @return
             */
            std::string readString(unsigned long length);

            /**
             * Reads characters until it reach a '\0'
             * @return
             */
            std::string readNextString();

            /**
             * Reads the next big endian unsigned int.
             * @return
             */
            uint32_t readNextBeUint();

            /**
             * Reads the next little endian unsigned int.
             * @return
             */
            uint32_t readNextLeUint();
            /**
             * Reads the next little endian int.
             * @return
             */
            uint64_t readNextBeUlong();

            /**
             * Reads the next little endian unsigned long.
             * @return
             */
            uint64_t readNextLeUlong();

            ledger::core::BigInt readNextBeBigInt(size_t bytes);
            ledger::core::BigInt readNextLeBigInt(size_t bytes);

            uint64_t readNextVarInt();
            std::string readNextVarString();

            std::vector<uint8_t> readUntilEnd();

            /**
            * Reset cursor position and offset to 0
            */
            void reset();

            /**
             * Gets current cursor position
             * @return Current byte offset.
             */
            unsigned long getCursor() const;
            /**
             * Checks if the reader can read further.
             * @return True if it remains bytes to read, false otherwise.
             */
            bool hasNext() const;
            /**
             * Gets the number of bytes before the end.
             * @return The number of bytes before the end.
             */
            unsigned long available() const;


        private:
            std::vector<uint8_t> _bytes;
            unsigned long _cursor;
            unsigned long _offset;
            unsigned long _length;
        };
    }
}
