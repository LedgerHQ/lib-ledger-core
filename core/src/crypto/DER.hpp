/*
 *
 * DER.hpp
 * ledger-core
 *
 * Created by Axel Haustant on 01/10/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#ifndef LEDGER_CORE_DER_HPP
#define LEDGER_CORE_DER_HPP

#include <cstdint>
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {

        namespace ASN1 {
            static const uint8_t SEQUENCE = 0x30;
            static const uint8_t INTEGER = 0x02;
        };

        /**
         * @brief Represent a DER packed signature
         * 
         */
        struct DER {
            std::vector<uint8_t> r;
            std::vector<uint8_t> s;

            /**
             * @brief Serialize the DER signature as a raw bytes sequence
             * 
             * @return std::vector<uint8_t> 
             */
            std::vector<uint8_t> toBytes();

            /**
             * @brief Parse a raw DER signature
             * 
             * @param raw A DER signature as bytes
             * @return DER A Parsed DER
             */
            static DER fromRaw(const std::vector<uint8_t> &raw);
        };
    }
}


#endif //LEDGER_CORE_DER_HPP
