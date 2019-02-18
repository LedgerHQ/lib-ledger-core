/*
 *
 * Bech32
 *
 * Created by El Khalil Bellakrid on 13/02/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#include "Bech32.h"
#include <collections/vector.hpp>

namespace ledger {
    namespace core {

        // The Bech32 character set for encoding.
        const char* charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

        // The Bech32 character set for decoding.
        const int8_t charsetRev[128] = {
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                15, -1, 10, 17, 21, 20, 26, 30,  7,  5, -1, -1, -1, -1, -1, -1,
                -1, 29, -1, 24, 13, 25,  9,  8, 23, -1, 18, 22, 31, 27, 19, -1,
                1,  0,  3, 16, 11, 28, 12, 14,  6,  4,  2, -1, -1, -1, -1, -1,
                -1, 29, -1, 24, 13, 25,  9,  8, 23, -1, 18, 22, 31, 27, 19, -1,
                1,  0,  3, 16, 11, 28, 12, 14,  6,  4,  2, -1, -1, -1, -1, -1
        };

        // Verify a checksum.
        bool Bech32::verifyChecksum(const std::vector<uint8_t>& values,
                                    const Bech32Parameters::Bech32Struct& params) {
            return polymod(vector::concat(expandHrp(params.hrp), values), params) == 1;
        }

        // Create a checksum.
        std::vector<uint8_t> Bech32::createChecksum(const std::vector<uint8_t>& values,
                                                    const Bech32Parameters::Bech32Struct& params) {
            std::vector<uint8_t> enc = vector::concat(expandHrp(params.hrp), values);
            enc.resize(enc.size() + params.checksumSize);
            uint64_t mod = polymod(enc, params) ^ 1;
            std::vector<uint8_t> ret;
            ret.resize(params.checksumSize);
            for (size_t i = 0; i < params.checksumSize; ++i) {
                ret[params.checksumSize - 1 - i] = mod & 31;
                mod = mod >> 5;
            }
            return ret;
        }
        
        std::string Bech32::encode(const std::vector<uint8_t>& values) {
            // Values here should be concatenation of version + hash
            std::vector<uint8_t> checksum = createChecksum(values, _bech32Params);
            std::vector<uint8_t> combined = vector::concat(values, checksum);
            std::string ret = _bech32Params.hrp + _bech32Params.separator;
            ret.reserve(ret.size() + combined.size());
            for (size_t i = 0; i < combined.size(); ++i) {
                ret += charset[combined[i]];
            }
            return ret;
        }

        std::pair<std::string, std::vector<uint8_t>>
        Bech32::decode(const std::string& str) {
            bool lower = false, upper = false;
            bool ok = true;
            for (size_t i = 0; ok && i < str.size(); ++i) {
                unsigned char c = str[i];
                if (c < 33 || c > 126) ok = false;
                if (c >= 'a' && c <= 'z') lower = true;
                if (c >= 'A' && c <= 'Z') upper = true;
            }
            if (lower && upper) ok = false;
            size_t pos = str.rfind(_bech32Params.separator);
            if (ok && str.size() <= 90 && pos != str.npos && pos >= 1 && pos + 7 <= str.size()) {
                std::vector<uint8_t> values;
                values.resize(str.size() - 1 - pos);
                for (size_t i = 0; i < str.size() - 1 - pos; ++i) {
                    unsigned char c = str[i + pos + 1];
                    if (charsetRev[c] == -1) ok = false;
                    values[i] = charsetRev[c];
                }
                if (ok) {
                    std::string hrp;
                    for (size_t i = 0; i < pos; ++i) {
                        hrp += toLowerCase(str[i]);
                    }
                    if (verifyChecksum(values, _bech32Params)) {
                        return std::make_pair(hrp, std::vector<uint8_t>(values.begin(), values.end() - 6));
                    }
                }
            }
            return std::make_pair(std::string(), std::vector<uint8_t>());
        }

        // Convert to lower case.
        unsigned char Bech32::toLowerCase(unsigned char c) {
            return (c >= 'A' && c <= 'Z') ? (c - 'A') + 'a' : c;
        }

        // Convert from one power-of-2 number base to another. */
        bool Bech32::convertBits(const std::vector<uint8_t>& in,
                                 int fromBits,
                                 int toBits,
                                 bool pad,
                                 std::vector<uint8_t>& out) {
            int acc = 0;
            int bits = 0;
            const int maxv = (1 << toBits) - 1;
            const int max_acc = (1 << (fromBits + toBits - 1)) - 1;
            for (size_t i = 0; i < in.size(); ++i) {
                int value = in[i];
                acc = ((acc << fromBits) | value) & max_acc;
                bits += fromBits;
                while (bits >= toBits) {
                    bits -= toBits;
                    out.push_back((acc >> bits) & maxv);
                }
            }
            if (pad) {
                if (bits) out.push_back((acc << (toBits - bits)) & maxv);
            } else if (bits >= fromBits || ((acc << (toBits - bits)) & maxv)) {
                return false;
            }
            return true;
        }

        std::vector<uint8_t>
        Bech32::segwitScriptPubkey(int witnessVersion,
                                   const std::vector<uint8_t>& witnessProg) {
            std::vector<uint8_t> ret;
            ret.push_back(witnessVersion ? (0x80 | witnessVersion) : 0);
            ret.push_back(witnessProg.size());
            ret.insert(ret.end(), witnessProg.begin(), witnessProg.end());
            return ret;
        }

    }
}