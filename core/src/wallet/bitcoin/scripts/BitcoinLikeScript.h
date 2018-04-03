/*
 *
 * BitcoinLikeScript.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/04/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#ifndef LEDGER_CORE_BITCOINLIKESCRIPT_H
#define LEDGER_CORE_BITCOINLIKESCRIPT_H

#include <utils/Try.hpp>
#include <utils/Either.hpp>
#include "operators.h"
#include <list>

namespace ledger {
    namespace core {

        using BitcoinLikeOpCode = btccore::opcodetype;
        using BitcoinLikePushedByte = std::vector<uint8_t>;

        using BitcoinLikeScriptChunk = Either<BitcoinLikePushedByte, BitcoinLikeOpCode>;

        class BitcoinLikeScript {
        public:
            BitcoinLikeScript() {};

            BitcoinLikeScript& operator<<(btccore::opcodetype op_code);
            BitcoinLikeScript& operator<<(const std::vector<uint8_t>& bytes);
            std::string toString() const;
            std::vector<uint8_t> serialize() const;
            const std::list<BitcoinLikeScriptChunk>& toList();

            static Try<BitcoinLikeScript> parse(const std::vector<uint8_t>& script);

        private:
            std::list<BitcoinLikeScriptChunk> _chunks;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKESCRIPT_H
