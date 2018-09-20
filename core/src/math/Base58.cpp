/*
 *
 * Base58
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
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
#include "Base58.hpp"
#include "BigInt.h"
#include <sstream>
#include "../collections/vector.hpp"
#include <crypto/HashAlgorithm.h>
#include <utils/hex.h>
#include <functional>

static const std::string DIGITS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const ledger::core::BigInt V_58(58);

static void _encode(const ledger::core::BigInt& v, std::stringstream& ss) {
    if (v == ledger::core::BigInt::ZERO) {
        return ;
    }
    auto r = (v % V_58).toUnsignedInt();
    _encode(v / V_58, ss);
    ss << DIGITS[r];
}

std::string ledger::core::Base58::encode(const std::vector<uint8_t> &bytes) {
    BigInt intData(bytes.data(), bytes.size(), false);
    std::stringstream ss;

    for (auto i = 0; i < bytes.size() && bytes[i] == 0; i++) {
        ss << DIGITS[0];
    }
    _encode(intData, ss);
    return ss.str();
}

std::string ledger::core::Base58::encodeWithChecksum(const std::vector<uint8_t> &bytes, const std::string &networkIdentifier) {
    return encode(vector::concat<uint8_t>(bytes, computeChecksum(bytes, networkIdentifier)));
}

std::vector<uint8_t> ledger::core::Base58::decode(const std::string &str) throw(ledger::core::Exception) {
    BigInt intData(0);
    std::vector<uint8_t> prefix;

    for (auto& c : str) {
        if (c == '1' && intData == BigInt::ZERO) {
            prefix.push_back(0);
        } else {
            auto digitIndex = DIGITS.find(c);
            if (digitIndex != std::string::npos) {
                intData = (intData * V_58) + BigInt((unsigned int)digitIndex);
            } else {
                throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Invalid base 58 format");
            }
        }
    }
    return vector::concat(prefix, intData.toByteArray());
}

std::vector<uint8_t> ledger::core::Base58::computeChecksum(const std::vector<uint8_t> &bytes, const std::string &networkIdentifier) {
    ledger::core::HashAlgorithm hashAlgorithm(networkIdentifier);
    auto hash = hashAlgorithm.bytesToBytesHash(bytes);
    auto doubleHash = hashAlgorithm.bytesToBytesHash(hash);
    return std::vector<uint8_t>(doubleHash.begin(), doubleHash.begin() + 4);
}

ledger::core::Try<std::vector<uint8_t>> ledger::core::Base58::checkAndDecode(const std::string &str, const std::string &networkIdentifier) {
    return Try<std::vector<uint8_t>>::from([&] () {
        auto decoded = decode(str);
        //Check decoded address size
        if (decoded.size() <= 4) {
            throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Invalid address : Invalid base 58 format");
        }
        std::vector<uint8_t> data(decoded.begin(), decoded.end() - 4);
        std::vector<uint8_t> checksum(decoded.end() - 4, decoded.end());
        auto chks = computeChecksum(data, networkIdentifier);
        if (checksum != chks) {
            throw Exception(api::ErrorCode::INVALID_CHECKSUM, "Base 58 invalid checksum");
        }
        return data;
    });
}