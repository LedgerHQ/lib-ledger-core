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


#include "SHA512256.hpp"
#include <utils/Exception.hpp>
#include <utils/hex.hpp>
#include <math/BaseConverter.hpp>

#include <openssl/sha.h>


namespace ledger {
namespace core {
namespace algorand {

    // Stolen from OpenSSL 1.1.1:
    // https://github.com/openssl/openssl/blob/4bed94f0c11ef63587c6b2edb03c3c438e221604/crypto/sha/sha512.c#L81
    static void sha512_256_init(SHA512_CTX *c)
    {
        c->h[0] = U64(0x22312194fc2bf72c);
        c->h[1] = U64(0x9f555fa3c84c64c2);
        c->h[2] = U64(0x2393b86b6f53b151);
        c->h[3] = U64(0x963877195940eabd);
        c->h[4] = U64(0x96283ee2a88effe3);
        c->h[5] = U64(0xbe5e1e2553863992);
        c->h[6] = U64(0x2b0199fc2c85b8aa);
        c->h[7] = U64(0x0eb72ddc81c52ca2);

        c->Nl = 0;
        c->Nh = 0;
        c->num = 0;
        c->md_len = SHA256_DIGEST_LENGTH;
    }

    std::string SHA512256::stringToHexHash(const std::string &input) {
        return hex::toString(stringToBytesHash(input));
    }

    std::string SHA512256::bytesToHexHash(const std::vector<uint8_t> &bytes) {
        return hex::toString(bytesToBytesHash(bytes));
    }

    std::vector<uint8_t> SHA512256::stringToBytesHash(const std::string &input) {
        return dataToBytesHash(reinterpret_cast<const uint8_t*>(input.c_str()), input.size());
    }

    std::vector<uint8_t> SHA512256::bytesToBytesHash(const std::vector<uint8_t> &bytes) {
        return dataToBytesHash(bytes.data(), bytes.size());
    }

    std::vector<uint8_t> SHA512256::dataToBytesHash(const uint8_t *data, size_t size) {

        uint8_t hash[SHA256_DIGEST_LENGTH];
        SHA512_CTX sha512;
        sha512_256_init(&sha512);
        SHA512_Update(&sha512, data, size);
        SHA512_Final(hash, &sha512);
        return std::vector<uint8_t >(hash, hash + SHA256_DIGEST_LENGTH);

    }

}
}
}