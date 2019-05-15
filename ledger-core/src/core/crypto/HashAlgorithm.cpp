/*
 *
 * HashAlgorithm
 *
 * Created by El Khalil Bellakrid on 19/09/2018.
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

#include <core/crypto/HashAlgorithm.h>
#include <core/crypto/SHA256.hpp>
#include <core/crypto/BLAKE.h>
#include <core/utils/hex.h>

namespace ledger {
    namespace core {
        HashAlgorithm::HashAlgorithm(const std::string &networkIdentifier): _identifier(networkIdentifier)
        {}

        std::string HashAlgorithm::stringToHexHash(const std::string &input) {
            return hex::toString(stringToBytesHash(input));
        }

        std::string HashAlgorithm::bytesToHexHash(const std::vector<uint8_t> &bytes) {
            return hex::toString(bytesToBytesHash(bytes));
        }

        std::vector<uint8_t> HashAlgorithm::stringToBytesHash(const std::string &input) const {
            if (_identifier == "dcr") {
                return BLAKE::stringToBytesHash(input);
            }
            return SHA256::stringToBytesHash(input);
        }

        std::vector<uint8_t> HashAlgorithm::bytesToBytesHash(const std::vector<uint8_t> &bytes) const {
            if (_identifier == "dcr") {
                return BLAKE::bytesToBytesHash(bytes);
            }
            return SHA256::bytesToBytesHash(bytes);
        }
    }
}
