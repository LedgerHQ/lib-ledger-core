/*
 *
 * RLPEncoder
 *
 * Created by El Khalil Bellakrid on 24/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include <core/bytes/RLP/RLPEncoder.hpp>

/*
 * Reursive Length Prefix Encoder
 * Reference: https://github.com/ethereum/wiki/wiki/RLP
 */

namespace ledger {
    namespace core {
        std::vector<uint8_t> RLPEncoder::encodeLength(uint32_t length, uint8_t offset, BytesWriter &out) {
            if (length < 56) {
                out.writeVarInt(length + offset);
            } else {
                std::vector<uint8_t> binary;
                RLPEncoder::toBinary(length, binary);
                out.writeVarInt(binary.size() + offset + 55);
                out.writeByteArray(binary);
            }
            return out.toByteArray();
        };

        void RLPEncoder::toBinary(uint32_t length, std::vector<uint8_t> &out) {
            if (length == 0) {
                return;
            } else {
                toBinary(length / 256, out);
                out.push_back(length % 256);
            }
        };
    }
}
