/*
 *
 * BLAKE
 *
 * Created by El Khalil Bellakrid on 18/09/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#ifndef LEDGER_CORE_BLAKE_H
#define LEDGER_CORE_BLAKE_H

#include <vector>
#include <string>

#include <openssl/opensslv.h>

extern "C" {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#include <blake2b.h>
#else
#include <blake2_local.h>
#endif
}

namespace ledger {
    namespace core {
        class BLAKE {
        public:
            static std::vector<uint8_t> blake256(const std::vector<uint8_t>& data);
            static std::vector<uint8_t> blake224(const std::vector<uint8_t>& data);
            static std::vector<uint8_t> blake2b(const std::vector<uint8_t>& data, size_t outLength = BLAKE2B_OUTBYTES, size_t offset = 0);
            static std::vector<uint8_t> stringToBytesHash(const std::string& input);
            static std::vector<uint8_t> bytesToBytesHash(const std::vector<uint8_t>& bytes);
        };
    }
}

#endif //LEDGER_CORE_BLAKE_H
