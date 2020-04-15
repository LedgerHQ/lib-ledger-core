/*
 *
 * Block
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/07/2017.
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
#ifndef LEDGER_CORE_BLOCK_H
#define LEDGER_CORE_BLOCK_H

#include <chrono>
#include <string>
#include <api/Block.hpp>

namespace ledger {
    namespace core {
        struct Block {
            std::string hash;
            uint64_t height;
            std::chrono::system_clock::time_point time;
            std::string currencyName;

            Block() {};
            // TODO : MARK AS EXPLICIT
            // this is implicit because of the raw count
            // of compilation errors to fight through. Once all errors are dealt with,
            // make this constructor explicit to avoid implicit conversions
            Block(const api::Block &apiBlock) :
                hash(apiBlock.blockHash),
                height(apiBlock.height),
                time(apiBlock.time),
                currencyName(apiBlock.currencyName){};
            std::string getUid() const;
            api::Block toApiBlock() const;
        };
    }
}


#endif //LEDGER_CORE_BLOCK_H
