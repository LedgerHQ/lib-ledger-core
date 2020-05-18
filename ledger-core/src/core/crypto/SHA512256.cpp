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
#include <core/utils/Exception.hpp>

#include <openssl/evp.h>

namespace ledger {
    namespace core {
        std::string SHA512256::stringToHexHash(const std::string &input) {
            return hex::toString(stringToBytesHash(input));
        }

        std::string SHA512256::bytesToHexHash(const std::vector<uint8_t> &bytes) {
            return hex::toString(bytesToBytesHash(bytes));
        }

        std::vector<uint8_t> SHA512256::dataToBytesHash(const void *data, size_t size) {
#if OPENSSL_VERSION_NUMBER < 0x10101000L
            throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION,
                                 "SHA512-256 requires OpenSSL 1.1.1 or above.");
#else
            uint8_t hash[EVP_MAX_MD_SIZE];
            uint32_t md_len;

            OpenSSL_add_all_digests();
            EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
            const EVP_MD *md = EVP_get_digestbyname("sha512-256");
            EVP_DigestInit_ex(mdctx, md, NULL);
            EVP_DigestUpdate(mdctx, data, size);
            EVP_DigestFinal_ex(mdctx, hash, &md_len);
            EVP_MD_CTX_destroy(mdctx);
            EVP_cleanup();
            return std::vector<uint8_t >(hash, hash + md_len);
#endif
        }

        std::vector<uint8_t> SHA512256::stringToBytesHash(const std::string &input) {
            return dataToBytesHash(input.c_str(), input.size());
        }

        std::vector<uint8_t> SHA512256::bytesToBytesHash(const std::vector<uint8_t> &bytes) {
            return dataToBytesHash(bytes.data(), bytes.size());
        }
    }
}
