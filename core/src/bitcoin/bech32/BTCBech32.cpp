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

namespace ledger {
    namespace core {
        uint64_t BTCBech32::polymod(const std::vector<uint8_t>& values,
                                    const Bech32Parameters::Bech32Struct& params) {
            uint32_t chk = 1;
            for (size_t i = 0; i < values.size(); ++i) {
                uint8_t top = chk >> 25;
                chk = (chk & 0x1ffffff) << 5 ^ values[i];
                auto index = 0;
                for (auto& gen : params.generator) {
                    chk ^= (-((top >> index) & 1) & gen);
                    index++;
                }
            }
            return chk;
        }

        std::vector<uint8_t> BTCBech32::expandHrp(const std::string& hrp) {
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
                                      const std::vector<uint8_t>& version) {
            std::vector<uint8_t> data(hash);
            int fromBits = 8, toBits = 5;
            bool pad = true;
            std::vector<uint8_t> converted;
            converted.insert(converted.end(), version.begin(), version.end());
            Bech32::convertBits(data, fromBits, toBits, pad, converted);
            return Bech32::encode(converted);
        }
    }
}