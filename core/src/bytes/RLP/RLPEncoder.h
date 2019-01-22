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


#ifndef LEDGER_CORE_RLPENCODER_H
#define LEDGER_CORE_RLPENCODER_H

#include <vector>
#include "../BytesWriter.h"

namespace ledger {
    namespace core {
        class RLPEncoder : public virtual std::enable_shared_from_this<RLPEncoder>{
        public:
            //TODO: replace append by pushList or pushString
            virtual std::vector<uint8_t> encode() = 0;
            virtual void append(const std::string &str) = 0;
            virtual void append(const std::vector<uint8_t> &data) = 0;
            virtual void append(const std::shared_ptr<RLPEncoder> &child) = 0;
            virtual std::string toString() = 0;
            virtual bool isList() = 0;
            virtual std::vector<std::shared_ptr<RLPEncoder>> getChildren() = 0;
            static std::vector<uint8_t> encodeLength(uint32_t length, uint8_t offset, BytesWriter &out);
            static void toBinary(uint32_t length, std::vector<uint8_t> &out);
        };
    }
}


#endif //LEDGER_CORE_RLPENCODER_H
