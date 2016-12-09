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
#include "AES256.hpp"
#include <openssl/aes.h>
#include <cstdio>
#include <cstring>

const uint32_t ledger::core::AES256::BLOCK_SIZE = AES_BLOCK_SIZE;

std::vector<uint8_t> ledger::core::AES256::encrypt(const std::vector<uint8_t> &IV, const std::vector<uint8_t> &key,
                                                   const std::vector<uint8_t> &data) {
    const size_t encslength = ((data.size() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    uint8_t enc_out[encslength];
    ::memset(enc_out, 0, sizeof(enc_out));

    uint8_t iv[IV.size()];
    ::memcpy(iv, IV.data(), IV.size());

    AES_KEY enc_key;
    AES_set_encrypt_key(key.data(), key.size() * 8, &enc_key);
    AES_cbc_encrypt(data.data(), enc_out, data.size(), &enc_key, iv, AES_ENCRYPT);

    return std::vector<uint8_t>(enc_out, enc_out + encslength);
}

std::vector<uint8_t> ledger::core::AES256::decrypt(const std::vector<uint8_t> &IV, const std::vector<uint8_t> &key,
                                                   const std::vector<uint8_t> &data) {
    /* init vector */
    unsigned char iv_enc[AES_BLOCK_SIZE], iv_dec[AES_BLOCK_SIZE];

    // buffers for encryption and decryption
    const size_t inputslength = data.size();
    unsigned char dec_out[inputslength];
    ::memset(dec_out, 0, sizeof(dec_out));

    uint8_t iv[IV.size()];
    ::memcpy(iv, IV.data(), IV.size());

    AES_KEY dec_key;
    AES_set_decrypt_key(key.data(), key.size() * 8, &dec_key);
    AES_cbc_encrypt(data.data(), dec_out, data.size(), &dec_key, iv, AES_DECRYPT);

    return std::vector<uint8_t>(dec_out, dec_out + inputslength);
}
