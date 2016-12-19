/*
 *
 * HMACSHA256
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/12/2016.
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

#include "HMAC.hpp"
#include <openssl/hmac.h>
#include <openssl/sha.h>

std::vector<uint8_t> ledger::core::HMAC::sha256(const std::vector<uint8_t>& key,
                                                    const std::vector<uint8_t>& data) {
    auto len = SHA256_DIGEST_LENGTH;
    uint8_t hash[len];
    HMAC_CTX hmac;
    HMAC_CTX_init(&hmac);
    HMAC_Init_ex(&hmac, key.data(), key.size(), EVP_sha256(), NULL);
    HMAC_Update(&hmac, data.data(), data.size());
    HMAC_Final(&hmac, hash, (unsigned int *)(&len));
    HMAC_cleanup(&hmac);
    return std::vector<uint8_t>(hash, hash + SHA256_DIGEST_LENGTH);
}

std::vector<uint8_t> ledger::core::HMAC::sha512(const std::vector<uint8_t>& key,
                                                    const std::vector<uint8_t>& data) {
    auto len = SHA512_DIGEST_LENGTH;
    uint8_t hash[len];
    HMAC_CTX hmac;
    HMAC_CTX_init(&hmac);
    HMAC_Init_ex(&hmac, key.data(), key.size(), EVP_sha512(), NULL);
    HMAC_Update(&hmac, data.data(), data.size());
    HMAC_Final(&hmac, hash, (unsigned int *)(&len));
    HMAC_cleanup(&hmac);
    return std::vector<uint8_t>(hash, hash + SHA512_DIGEST_LENGTH);
}
