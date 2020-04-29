/*
 * SHA512256
 *
 * Created by Hakim Aammar on 20/04/2020.
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


#include <core/crypto/SHA512256.hpp>
#include <core/utils/Hex.hpp>

#include <openssl/sha.h>

namespace ledger {
    namespace core {
        std::string SHA512256::stringToHexHash(const std::string &input) {
            return hex::toString(stringToBytesHash(input));
        }

        std::string SHA512256::bytesToHexHash(const std::vector<uint8_t> &bytes) {
            return hex::toString(bytesToBytesHash(bytes));
        }

        std::vector<uint8_t> SHA512256::dataToBytesHash(const void *data, size_t size) {
            /*
            // FIXME Use SHA-512/256 instead of SHA-512
            uint8_t hash[SHA512256_DIGEST_LENGTH];
            SHA512256_CTX sha512;
            SHA512256_Init(&sha512);
            SHA512256_Update(&sha512, data, size);
            SHA512256_Final(hash, &sha512);
            return std::vector<uint8_t >(hash, hash + SHA512_DIGEST_LENGTH);
            /*/
            uint8_t hash[SHA512_DIGEST_LENGTH];
            SHA512_CTX sha512;
            SHA512_Init(&sha512);
            SHA512_Update(&sha512, data, size);
            SHA512_Final(hash, &sha512);
            return std::vector<uint8_t >(hash, hash + SHA512_DIGEST_LENGTH);
            //*/
        }

        std::vector<uint8_t> SHA512256::stringToBytesHash(const std::string &input) {
            return dataToBytesHash(input.c_str(), input.size());
        }

        std::vector<uint8_t> SHA512256::bytesToBytesHash(const std::vector<uint8_t> &bytes) {
            return dataToBytesHash(bytes.data(), bytes.size());
        }
    }
}
