/*
 *
 * SHA256
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
#include "SHA256.hpp"
#include "../utils/hex.h"
#include <openssl/sha.h>

namespace ledger {
    namespace core {
        std::string SHA256::stringToHexHash(const std::string &input) {
            return hex::toString(SHA256::stringToBytesHash(input));
        }

        std::string SHA256::bytesToHexHash(const std::vector<uint8_t> &bytes) {
            return hex::toString(SHA256::bytesToBytesHash(bytes));
        }

        std::vector<uint8_t> SHA256::dataToBytesHash(const void *data, size_t size) {
            std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, data, size);
            SHA256_Final(&hash[0], &sha256);
            return hash;
        }
        
        std::vector<uint8_t> SHA256::stringToBytesHash(const std::string &input) {
            return dataToBytesHash(input.c_str(), input.size());
        }

        std::vector<uint8_t> SHA256::bytesToBytesHash(const std::vector<uint8_t> &bytes) {
            return dataToBytesHash(bytes.data(), bytes.size());
        }
    }
}
