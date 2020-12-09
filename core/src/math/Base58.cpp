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
#include <sstream>
#include "../collections/vector.hpp"
#include <crypto/HashAlgorithm.h>
#include <utils/hex.h>
#include <functional>
#include <crypto/Keccak.h>

using namespace ledger::core;
static const std::string DIGITS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static std::string getNetworkIdentifier(const std::shared_ptr<api::DynamicObject> &config) {
    return config->getString("networkIdentifier").value_or("");
}

static std::string getNetworkBase58Dictionary(const std::shared_ptr<api::DynamicObject> &config) {
    return config->getString("base58Dictionary").value_or(DIGITS);
}

static bool shouldUseNetworkBase58Dictionary(const std::shared_ptr<api::DynamicObject> &config) {
    return config->getBoolean("useNetworkDictionary").value_or(false);
}

// refer to https://bitcoin.stackexchange.com/questions/76480/encode-decode-base-58-c
std::string ledger::core::Base58::encode(const std::vector<uint8_t> &bytes,
                                         const std::shared_ptr<api::DynamicObject> &config) {
    auto base58Dictionary = getNetworkBase58Dictionary(config);
    std::string result;
    const double iFactor = 1.36565823730976103695740418120764243208481439700722980119458355862779176747360903943915516885072037696111192757109;
    int len = bytes.size();
    int zeros = 0, length = 0, pbegin = 0, pend;
    pend = len;
    while (pbegin != pend && !bytes[pbegin]) pbegin = ++zeros;
    const int size = 1 + iFactor * (double)(pend - pbegin);
    unsigned char* b58 = new unsigned char[size];
    for (int i = 0; i < size; i++) b58[i] = 0;
    while (pbegin != pend) {
        unsigned int carry = bytes[pbegin];
        int i = 0;
        for (int it1 = size - 1; (carry || i < length) && (it1 != -1); it1--, i++) {
            carry += 256 * b58[it1];
            b58[it1] = carry % 58;
            carry /= 58;
        }
        if (carry) return result;
        length = i;
        pbegin++;
    }
    int it2 = size - length;
    while ((it2 - size) && !b58[it2]) it2++;

    int ri = 0;
    while (ri < zeros) { result += base58Dictionary[0]; ri++; }
    for (; it2 < size; ++it2) result += base58Dictionary[b58[it2]];
    delete[] b58;
    return result;
}

std::string ledger::core::Base58::encodeWithChecksum(const std::vector<uint8_t> &bytes,
                                                     const std::shared_ptr<api::DynamicObject> &config) {

    return encode(vector::concat<uint8_t>(bytes, computeChecksum(bytes, getNetworkIdentifier(config))), config);
}

std::string ledger::core::Base58::encodeWithEIP55(const std::vector<uint8_t> &bytes) {
    auto byteToDigitEIP55 = [](uint8_t byte, uint8_t against) -> char {
        bool uppercase = against >= 0x8;
        byte = (uint8_t) (0xF < byte ? 0xF : byte);
        if (byte < 0xA) {
            return '0' + byte;
        } else if (uppercase) {
            return (char)('A' + (byte - 0xA));
        } else {
            return (char)('a' + (byte - 0xA));
        }
    };

    auto keccakHash = Keccak::keccak256(hex::toString(bytes));

    std::string addressEIP55(bytes.size() * 2, '0');
    for (int i = 0; i < bytes.size(); i++) {
        addressEIP55[i * 2] = byteToDigitEIP55(bytes[i] >> 4, keccakHash[i] >> 4);
        addressEIP55[i * 2 + 1] = byteToDigitEIP55((uint8_t) (bytes[i] & 0xF), (uint8_t) (keccakHash[i] & 0xF));
    }
    return "0x" + addressEIP55;
}

std::string ledger::core::Base58::encodeWithEIP55(const std::string &address) {
    if (address.size() == 42) {
        //Address with 0x prefix
        return Base58::encodeWithEIP55(hex::toByteArray(address.substr(2, address.size() - 2)));
    } else if (address.size() == 40) {
        return Base58::encodeWithEIP55(hex::toByteArray(address));
    }
    throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Invalid base 58 format");
}

// refer to https://bitcoin.stackexchange.com/questions/76480/encode-decode-base-58-c
std::vector<uint8_t> ledger::core::Base58::decode(const std::string &str,
                                                  const std::shared_ptr<api::DynamicObject> &config) {
    auto useBase58Dict = shouldUseNetworkBase58Dictionary(config);
    auto base58Dictionary = useBase58Dict ? getNetworkBase58Dictionary(config) : DIGITS;
    int len = str.size();
    std::vector<uint8_t> result( len * 2 );
    result[0] = 0;
    int resultlen = 1;
    for (int i = 0; i < len; i++) {
        auto carry = base58Dictionary.find(str[i]);
        if (carry == std::string::npos) { throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Invalid base 58 format"); }
        for (int j = 0; j < resultlen; j++) {
            carry += (result[j]) * 58;
            result[j] = (unsigned char)(carry & 0xff);
            carry >>= 8;
        }
        while (carry > 0) {
            result[resultlen++] = carry & 0xff;
            carry >>= 8;
        }
    }

    for (int i = 0; i < len && str[i] == base58Dictionary[0]; i++)
        result[resultlen++] = 0;

    for (int i = resultlen - 1, z = (resultlen >> 1) + (resultlen & 1);
        i >= z; i--) {
        int k = result[i];
        result[i] = result[resultlen - i - 1];
        result[resultlen - i - 1] = k;
    }
    result.resize(resultlen);
    return result;
}

std::vector<uint8_t> ledger::core::Base58::computeChecksum(const std::vector<uint8_t> &bytes,
                                                           const std::string &networkIdentifier) {
    ledger::core::HashAlgorithm hashAlgorithm(networkIdentifier);
    auto hash = hashAlgorithm.bytesToBytesHash(bytes);
    auto doubleHash = hashAlgorithm.bytesToBytesHash(hash);
    return std::vector<uint8_t>(doubleHash.begin(), doubleHash.begin() + 4);
}

ledger::core::Try<std::vector<uint8_t>> ledger::core::Base58::checkAndDecode(const std::string &str,
                                                                             const std::shared_ptr<api::DynamicObject> &config) {
    return Try<std::vector<uint8_t>>::from([&] () {
        auto decoded = decode(str, config);
        //Check decoded address size
        if (decoded.size() <= 4) {
            throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Invalid address : Invalid base 58 format");
        }
        std::vector<uint8_t> data(decoded.begin(), decoded.end() - 4);
        std::vector<uint8_t> checksum(decoded.end() - 4, decoded.end());
        auto chks = computeChecksum(data, getNetworkIdentifier(config));
        if (checksum != chks) {
            throw Exception(api::ErrorCode::INVALID_CHECKSUM, "Base 58 invalid checksum");
        }
        return data;
    });
}