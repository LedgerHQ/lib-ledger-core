/*
 *
 * BTCBech32
 *
 * Created by El Khalil Bellakrid on 18/02/2019.
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

#include "BTCBech32.h"
#include <utils/Exception.hpp>
#include <collections/vector.hpp>

namespace ledger {
    namespace core {
        namespace {
            const uint32_t bech32mParam = 0x2bc830a3;
        }

        uint64_t BTCBech32::polymod(const std::vector<uint8_t>& values) const {
            uint32_t chk = 1;
            for (size_t i = 0; i < values.size(); ++i) {
                uint8_t top = chk >> 25;
                chk = (chk & 0x1ffffff) << 5 ^ values[i];
                auto index = 0;
                for (auto& gen : _bech32Params.generator) {
                    chk ^= (-((top >> index) & 1) & gen);
                    index++;
                }
            }
            return chk;
        }

        std::vector<uint8_t> BTCBech32::expandHrp(const std::string& hrp) const {
            std::vector<uint8_t> ret;
            ret.resize(hrp.size() * 2 + 1);
            for (size_t i = 0; i < hrp.size(); ++i) {
                unsigned char c = hrp[i];
                ret[i] = c >> 5;
                ret[i + hrp.size() + 1] = c & 0x1f;
            }
            ret[hrp.size()] = 0;
            return ret;
        }

        std::string BTCBech32::encode(const std::vector<uint8_t>& hash,
                                      const std::vector<uint8_t>& version) const {
            std::vector<uint8_t> data(hash);
            int fromBits = 8, toBits = 5;
            bool pad = true;
            std::vector<uint8_t> converted;
            converted.insert(converted.end(), version.begin(), version.end());
            Bech32::convertBits(data, fromBits, toBits, pad, converted);
            if (version.size() == 1) {
                if (version == _bech32Params.P2WPKHVersion || version == _bech32Params.P2WSHVersion) {
                    return encodeBech32(converted, 1);
                } else if (version[0] <= 16) {
                    return encodeBech32(converted, bech32mParam);
                }
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid Bech32 version value : must be in the range 0..16 inclusive");
            }
            throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid Bech32 version length : must be exactly 1 byte");
        }

        std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
        BTCBech32::decode(const std::string& str) const {
            auto decoded = decodeBech32Raw(str);

            if (decoded.second.size() < 1) {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid address : invalid Bech32 format");
            }

            std::vector<uint8_t> version{decoded.second[0]};

            if (decoded.second.size() < _bech32Params.checksumSize) {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid address : the address is less than checksum size");
            }

            if (version[0] == 0) {
                if (!verifyChecksum(decoded.second, 1)) {
                    throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Bech32 checksum verification failed");
                }
            } else if (version[0] <= 16) {
                if (!verifyChecksum(decoded.second, bech32mParam)) {
                    throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Bech32M checksum verification failed");
                }
            } else {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid Bech32 version value : must be in the range 0..16 inclusive");
            }

            // strip the checksum
            std::vector<uint8_t> decoded_address(decoded.second.begin(), decoded.second.end() - _bech32Params.checksumSize);

            if (decoded.first != _bech32Params.hrp) {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid address : Invalid Bech32 hrp");
            }
            std::vector<uint8_t> converted;
            int fromBits = 5, toBits = 8;
            bool pad = false;
            auto result = Bech32::convertBits(std::vector<uint8_t>(decoded_address.begin() + 1, decoded_address.end()),
                                              fromBits,
                                              toBits,
                                              pad,
                                              converted);

            if (!result || converted.size() < 2 || converted.size() > 40 || version.size() != 1) {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid address : Invalid Bech32 format");
            }

            if ((converted.size() == 20 && version == _bech32Params.P2WPKHVersion) ||
                (converted.size() == 32 && (version == _bech32Params.P2WSHVersion
                                            || version == _bech32Params.P2TRVersion))) {
                return std::make_pair(version, converted);
            }

            throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid address : Invalid Bech32 format, data length and version missmatch");
        }
    }
}
