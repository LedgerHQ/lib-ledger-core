/*
 *
 * Bytes
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/03/2017.
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
#ifndef LEDGER_CORE_BYTES_HPP
#define LEDGER_CORE_BYTES_HPP

#include "../utils/hex.h"
#include "Sequence.hpp"
#include "String.hpp"

namespace ledger {
    namespace core {

        class Bytes : public Array<uint8_t> {
          public:
            Bytes() : Array<uint8_t>(){};
            Bytes(const std::vector<uint8_t> &bytes) : Array<uint8_t>(bytes){

                                                       };

            String toAscii() const {
                return std::string((char *)getContainer().data(), size());
            }

            String toHex(bool uppercase = false) const {
                return hex::toString(getContainer(), uppercase);
            }

            static Bytes fromHex(const String &hex) {
                return hex::toByteArray(hex.str());
            }
        };
    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_BYTES_HPP
