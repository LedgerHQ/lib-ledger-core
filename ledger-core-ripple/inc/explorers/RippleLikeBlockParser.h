/*
 *
 * RippleLikeBlockParser
 *
 * Created by El Khalil Bellakrid on 07/01/2019.
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

#include <core/explorers/AbstractBlockParser.h>
#include <explorers/RippleLikeBlockchainExplorer.h>

namespace ledger {
    namespace core {
        class RippleLikeBlockParser : public AbstractBlockParser<RippleLikeBlockchainExplorer::Block> {
        public:
            RippleLikeBlockParser(std::string &lastKey) : _lastKey(lastKey) {};

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                if (getLastKey() == "ledger_index") {
                    std::string number(str, length);
                    BigInt value = BigInt::fromString(number);
                    _block->height = value.toUint64();
                }
                return true;
            }

            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                std::string value = std::string(str, length);
                if (getLastKey() == "ledger_hash") {
                    _block->blockHash = value;
                } else if (getLastKey() == "close_time_human") {
                    _block->time = DateUtils::fromJSON(value);
                }
                return true;
            }

        protected:
            std::string &getLastKey() override {
                return _lastKey;
            };
        private:
            std::string &_lastKey;
        };
    }
}
