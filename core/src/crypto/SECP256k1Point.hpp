/*
 *
 * SECP256k1Point
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/12/2016.
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
#ifndef LEDGER_CORE_SECP256K1POINT_HPP
#define LEDGER_CORE_SECP256K1POINT_HPP

#include <include/secp256k1.h>
#include "../math/BigInt.h"
#include <cstdint>

namespace ledger {
    namespace core {
        class SECP256k1Point {
        public:
            explicit SECP256k1Point(const std::vector<uint8_t>& p);
            SECP256k1Point generatorMultiply(const std::vector<uint8_t>& n) const;
            SECP256k1Point(SECP256k1Point&& p) = default;
            SECP256k1Point(const SECP256k1Point& p) = delete;
            SECP256k1Point& operator=(const SECP256k1Point& p) = delete;
            std::vector<uint8_t> toByteArray(bool compressed = true) const;
        private:
            using ContextUniqPtr = std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)>;
            static ContextUniqPtr _context;
            secp256k1_pubkey _pubKey;
        };
    }
}


#endif //LEDGER_CORE_SECP256K1POINT_HPP
