/*
 *
 * AES256
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/12/2016.
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
#ifndef LEDGER_CORE_AES256_HPP
#define LEDGER_CORE_AES256_HPP

#include "../api/RandomNumberGenerator.hpp"
#include <vector>

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER) && _MSC_VER <= 1900
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

namespace ledger {
    namespace core {
        class AES256 {
        public:
            static std::vector<uint8_t> encrypt(const std::vector<uint8_t>& IV, const std::vector<uint8_t>& key, const std::vector<uint8_t>& data);
            static std::vector<uint8_t> decrypt(const std::vector<uint8_t>& IV, const std::vector<uint8_t>& key, const std::vector<uint8_t>& data);
            static LIBCORE_EXPORT const uint32_t BLOCK_SIZE;
        };
    }
}


#endif //LEDGER_CORE_AES256_HPP
