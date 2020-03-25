/*
 *
 * CRC.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/02/2019.
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

#ifndef LEDGER_CORE_CRC_HPP
#define LEDGER_CORE_CRC_HPP

#include <cstdint>
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {

        class CRC {
        public:
            /**
            * The CRC profile holds all constants and parameters used by the CRC calcultation algorithm for 16bits versions
            */
            struct CRC16Profile;

            /**
             * CRC XMODEM profile (Poly: 0x1021, Init: 0x0000, RefIn: false, RefOut: false, XorOut: false)
             */
            static CRC16Profile XMODEM;

            /**
             * Calculate the cyclic redundancy check for the given bytes using a 16bits CRC profile.
             * @param bytes The data on which you want to compute the checksum.
             * @param profile The profile used by the algorithm.
             * @return The cyclic redundancy check of the given bytes.
             */
            static uint16_t calculate(const std::vector<uint8_t>& bytes, CRC16Profile& profile);

        };
    }
}


#endif //LEDGER_CORE_CRC_HPP
