/*
 *
 * hex
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2016.
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
#ifndef LEDGER_CORE_HEX_H
#define LEDGER_CORE_HEX_H

#include <memory>
#include <string>
#include <vector>

namespace ledger {
    namespace core {
        /**
         * Utility functions to deal with hexadecimal encoded string.
         */
        namespace hex {

            /**
             * Turns a single digit into its byte value.
             * @param c
             * @return
             */
            inline uint8_t digitToByte(char c);
            /**
             * Turns a single byte (the byte must be lower than 0x10) into a digit.
             * @throw std::invalid_argument When encountering an invalid character.
             * @return
             */
            inline char byteToDigit(uint8_t, bool uppercase);
            /**
             * Decodes the given string into a byte array.
             * @param hexString A string of bytes expressed in hexadecimal (e.g. "A0B1").
             * @throw std::invalid_argument When encountering an invalid character.
             * @return
             */
            std::vector<uint8_t> toByteArray(const std::string& hexString);
            /**
             * Encodes the given byte array into an hexadecimal string.
             * @param data A byte array
             * @param uppercase True if the output should be expressed in uppercase characters, false otherwise
             * @return
             */
            std::string toString(const std::vector<uint8_t>& data, bool uppercase);
            /**
             * Encodes the given byte array into an hexadecimal string (lowercase).
             * @param data
             * @return
             */
            std::string toString(const std::vector<uint8_t>& data);
        }

    }
}

#endif //LEDGER_CORE_HEX_H
