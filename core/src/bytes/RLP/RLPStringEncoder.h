/*
 *
 * RLPStringEncoder
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


#ifndef LEDGER_CORE_RLPSTRINGENCODER_H
#define LEDGER_CORE_RLPSTRINGENCODER_H

#include "RLPEncoder.h"
#include <string>
#include "../BytesWriter.h"
#include "../../utils/Exception.hpp"

namespace ledger {
    namespace core {
        class RLPStringEncoder final : public RLPEncoder {
        public:
            RLPStringEncoder(const std::string &data);
            RLPStringEncoder(const std::vector<uint8_t> &data);
            std::vector<uint8_t> encode() override ;
            void append(const std::string &str) override ;
            void append(const std::shared_ptr<RLPEncoder> &child) override ;
            bool isList() override ;
            std::string toString() override ;
            std::vector<std::shared_ptr<RLPEncoder>> getChildren() override ;
        private:
            std::vector<uint8_t> _data;
        };

    }
}

#endif //LEDGER_CORE_RLPSTRINGENCODER_H
