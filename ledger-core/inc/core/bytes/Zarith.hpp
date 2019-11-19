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

#pragma once

#include <algorithm>
#include <vector>

#include <core/bytes/BytesReader.hpp>
#include <core/utils/Hex.hpp>

namespace ledger {
    namespace core {
        namespace zarith {
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
            std::vector<uint8_t> zSerializeNumber(const std::vector<uint8_t> &inputData);

            std::vector<uint8_t> zParse(const std::vector<uint8_t> &inputData);

            // The mode where we force parser to continue parsing even when finding
            // a null bit was made just for testing purpose
            std::vector<uint8_t> zParse(BytesReader &reader);
        }
    }
}
