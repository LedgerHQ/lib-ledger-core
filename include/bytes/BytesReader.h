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
#ifndef LEDGER_CORE_BYTESREADER_H
#define LEDGER_CORE_BYTESREADER_H

#include <cstdint>
#include <array>
#include <vector>
#include "../math/BigInt.h"
#include "../ledger-core.h"

namespace ledger {

    namespace core {
        class BytesReader {

        public:
            BytesReader(uint8_t *bytes, unsigned long offset, unsigned long length) :
                    _bytes(bytes), _offset(offset), _length(length) {};

            BytesReader(std::vector<uint8_t> bytes) : BytesReader((uint8_t *) (bytes.data()), 0, bytes.size()) {};

            std::vector<uint8_t> read(size_t length);

            void seek(int length);

            std::string readString(size_t length);

            BigInt readLeBigInteger(size_t length);

            BigInt readBeBigInteger(size_t length);

            uint8_t readNextByte();

            unsigned int readNextBeUnsignedInt();

            unsigned int readNextLeUnsignedInt();

            int readNextBeInt();

            int readNextLeInt();

        private:
            const uint8_t *_bytes;
            unsigned long _offset;
            const unsigned long _length;
        };

    }
}
#endif //LEDGER_CORE_BYTESREADER_H
