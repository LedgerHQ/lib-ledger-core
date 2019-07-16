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


#include "BLAKE.h"
#include <blake256.h>

namespace ledger {
    namespace core {
        std::vector<uint8_t> BLAKE::blake256(const std::vector<uint8_t>& data) {
            auto len = 32;
            std::vector<uint8_t> hash(len);
            state blakeState;
            blake256_init(&blakeState);
            blake256_update(&blakeState, data.data(), data.size() * 8);
            blake256_final(&blakeState, hash.data());
            return hash;
        }

        std::vector<uint8_t> BLAKE::blake224(const std::vector<uint8_t>& data) {
            auto len = 32;
            std::vector<uint8_t> hash(len);
            state blakeState;
            blake224_init(&blakeState);
            blake224_update(&blakeState, data.data(), data.size() * 8);
            blake224_final(&blakeState, hash.data());
            return hash;
        }

        std::vector<uint8_t> BLAKE::blake2b(const std::vector<uint8_t>& data, size_t outLength, size_t offset) {
            std::vector<uint8_t> hash(outLength);
            BLAKE2B_CTX blakeState;
            BLAKE2b_Init_default(&blakeState, outLength);
            BLAKE2b_Update(&blakeState, data.data() + offset, (data.size() - offset));
            BLAKE2b_Final(hash.data(), &blakeState);
            return hash;
        }

        std::vector<uint8_t> BLAKE::stringToBytesHash(const std::string &input) {
            std::vector<uint8_t> vInput(input.begin(), input.end());
            return bytesToBytesHash(vInput);
        }

        std::vector<uint8_t> BLAKE::bytesToBytesHash(const std::vector<uint8_t> &bytes) {
            return blake256(bytes);
        }

    }
}
