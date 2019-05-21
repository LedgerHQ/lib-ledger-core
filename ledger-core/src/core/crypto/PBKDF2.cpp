/*
 *
 * PBKDF2
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/12/2016.
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
#include <openssl/evp.h>

#include <core/crypto/PBKDF2.hpp>

namespace ledger {
    namespace core {
        std::vector<uint8_t>
        PBKDF2::derive(const std::vector<uint8_t> &key, const std::vector<uint8_t> &salt, uint32_t iter,
                       size_t outLength) {
            std::vector<uint8_t> result(outLength);
            PKCS5_PBKDF2_HMAC_SHA1((const char *)key.data(), key.size(),
                                   salt.data(), salt.size(),
                                   iter, outLength,
                                   result.data());
            return result;
        }
    }
}
