/*
 *
 * OpenSSLRandomNumberGenerator
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
#include "OpenSSLRandomNumberGenerator.hpp"
#include <openssl/rand.h>

std::vector<uint8_t> OpenSSLRandomNumberGenerator::getRandomBytes(int32_t size) {
    uint8_t result[size];
    RAND_bytes(result, size);
    return std::vector<uint8_t>(result, result + size);
}

int32_t OpenSSLRandomNumberGenerator::getRandomInt() {
    auto bytes = getRandomBytes(sizeof(int32_t));
    int32_t result = 0;
    for (auto off = 0; off < sizeof(int32_t); off++) {
        result = (result << 8) + bytes[off];
    }
    return result;
}

int64_t OpenSSLRandomNumberGenerator::getRandomLong() {
    auto bytes = getRandomBytes(sizeof(int64_t));
    int64_t result = 0;
    for (auto off = 0; off < sizeof(int64_t); off++) {
        result = (result << 8) + bytes[off];
    }
    return result;
}

int8_t OpenSSLRandomNumberGenerator::getRandomByte() {
    auto bytes = getRandomBytes(sizeof(int8_t));
    int8_t result = 0;
    for (auto off = 0; off < sizeof(int8_t); off++) {
        result = (result << 8) + bytes[off];
    }
    return result;
}
