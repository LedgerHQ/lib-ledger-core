/*
 *
 * CosmosBech32
 *
 * Created by El Khalil Bellakrid on 15/06/2019.
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

#include <cosmos/bech32/CosmosBech32.hpp>

#include <core/utils/Exception.hpp>

namespace ledger {
    namespace core {
        std::pair<std::vector<uint8_t>, std::vector<uint8_t>> CosmosBech32::decode(const std::string& str) {
            auto decoded = decodeBech32(str);
            if (decoded.first != _bech32Params.hrp || decoded.second.size() < 1) {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid address : Invalid bech 32 format");
            }
            std::vector<uint8_t> converted;
            int fromBits = 5, toBits = 8;
            bool pad = false;
            auto result = Bech32::convertBits(std::vector<uint8_t>(decoded.second.begin() + _offsetConversion, decoded.second.end()),
                                              fromBits,
                                              toBits,
                                              pad,
                                              converted);
             if (!result || converted.size() < 2 ||
                converted.size() > 40 ||
                (decoded.second[0] == 0 && converted.size() != 20 && converted.size() != 32)) {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid address : Invalid bech 32 format");
            }
            std::vector<uint8_t> version{decoded.second[0]};
            return std::make_pair(version, converted);
        }

        uint64_t CosmosBech32::polymod(const std::vector<uint8_t>& values) {
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

        std::vector<uint8_t> CosmosBech32::expandHrp(const std::string& hrp) {
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

        std::string CosmosBech32::encode(const std::vector<uint8_t>& hash,
                                         const std::vector<uint8_t>& version) {
            std::vector<uint8_t> data(hash);
            int fromBits = 8, toBits = 5;
            bool pad = true;
            std::vector<uint8_t> converted;
            converted.insert(converted.end(), version.begin(), version.end());
            Bech32::convertBits(data, fromBits, toBits, pad, converted);
            return encodeBech32(converted);
        }
    }
}
