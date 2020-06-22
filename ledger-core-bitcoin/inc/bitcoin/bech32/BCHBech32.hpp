/*
 *
 * BCHBech32
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

#pragma once

#include <core/math/bech32/Bech32.hpp>
#include <bitcoin/bech32/BitcoinLikeBech32ParametersHelpers.hpp>

// Reference: https://github.com/bitcoincashjs/cashaddrjs
namespace ledger {
    namespace core {
        class BCHBech32 : public Bech32 {
        public:
            BCHBech32() : Bech32(bitcoin::getBech32Params("abc")) {};

            uint64_t polymod(const std::vector<uint8_t>& values) const override;

            std::vector<uint8_t> expandHrp(const std::string& hrp) const override;

            std::string encode(const std::vector<uint8_t>& hash,
                               const std::vector<uint8_t>& version) const override;

            std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
            decode(const std::string& str) const override;
        };
    }
}
