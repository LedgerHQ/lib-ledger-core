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

#pragma once

#include <cstdint>
#include <include/secp256k1.h>

#include <core/math/BigInt.hpp>

namespace ledger {
    namespace core {
        class SECP256k1Point {
        public:
            SECP256k1Point(const std::vector<uint8_t>& p);
            SECP256k1Point operator+(const SECP256k1Point& p) const;
            SECP256k1Point generatorMultiply(const std::vector<uint8_t>& n) const;
            SECP256k1Point(const SECP256k1Point& p);
            std::vector<uint8_t> toByteArray(bool compressed = true) const;
            SECP256k1Point& operator=(const SECP256k1Point& p);
            bool isAtInfinity() const;
            ~SECP256k1Point();
        protected:
            SECP256k1Point();
            void ensurePubkeyIsNotNull() const;

        private:
            secp256k1_context* _context;
            secp256k1_pubkey* _pubKey;
        };
    }
}