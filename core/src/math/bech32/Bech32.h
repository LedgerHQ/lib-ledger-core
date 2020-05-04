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


#ifndef LEDGER_CORE_BECH32_H
#define LEDGER_CORE_BECH32_H

// References:
// BIP173: https://github.com/bitcoin/bips/blob/master/bip-0173.mediawiki
// Implementation: https://github.com/sipa/bech32/tree/master/ref/c%2B%2B

#include <vector>
#include <string>
#include "Bech32Parameters.h"

// HRP refers to Human Readable Part
namespace ledger {
    namespace core {
        class Bech32 {
        public:
            Bech32(Bech32Parameters::Bech32Struct bech32Params) : _bech32Params(bech32Params) {}
            // Find the polynomial with value coefficients mod the generator as 64-bit.
            virtual uint64_t polymod(const std::vector<uint8_t>& values) const = 0;

            // Expand a HRP for use in checksum computation.
            virtual std::vector<uint8_t> expandHrp(const std::string& hrp) const = 0;

            bool verifyChecksum(const std::vector<uint8_t>& values) const;

            std::vector<uint8_t> createChecksum(const std::vector<uint8_t>& values) const;

            virtual std::string encode(const std::vector<uint8_t>& hash,
                                       const std::vector<uint8_t>& version) const = 0;

            // Decode from bech32 address
            // @return pair<hrp, hash>
            std::pair<std::string, std::vector<uint8_t>>
            decodeBech32(const std::string& str) const;
            // @return tuple<witnessVersion, hash>
            virtual std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
            decode(const std::string& str) const = 0;

            static unsigned char toLowerCase(unsigned char c);

            static bool convertBits(const std::vector<uint8_t>& in,
                                    int fromBits,
                                    int toBits,
                                    bool pad,
                                    std::vector<uint8_t>& out);

            Bech32Parameters::Bech32Struct getBech32Params() const {
                return _bech32Params;
            }

        protected:
            std::string encodeBech32(const std::vector<uint8_t>& values) const;
            Bech32Parameters::Bech32Struct _bech32Params;
        };
    }
}
#endif //LEDGER_CORE_BECH32_H
