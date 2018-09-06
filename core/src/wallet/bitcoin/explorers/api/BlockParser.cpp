/*
 *
 * BlockParser
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/03/2017.
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
#include "BlockParser.hpp"
#include "utils/DateUtils.hpp"

namespace ledger {
    namespace core {

        bool BlockParser::Null() {
            return true;
        }

        bool BlockParser::Bool(bool b) {
            return true;
        }

        bool BlockParser::Int(int i) {
            return true;
        }

        bool BlockParser::Uint(unsigned i) {
            return true;
        }

        bool BlockParser::Int64(int64_t i) {
            return true;
        }

        bool BlockParser::Uint64(uint64_t i) {
            return true;
        }

        bool BlockParser::Double(double d) {
            return true;
        }

        bool BlockParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_lastKey == "height") {
                std::string number(str, length);
                BigInt value = BigInt::fromString(number);
                _block->height = value.toUint64();
            }
            return true;
        }

        bool BlockParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            std::string value = std::string(str, length);
            if (_lastKey == "hash") {
                _block->hash = value;
            } else if (_lastKey == "time") {
                _block->time = DateUtils::fromJSON(value);
            }
            return true;
        }

        bool BlockParser::StartObject() {
            return true;
        }

        bool BlockParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool BlockParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool BlockParser::StartArray() {
            return true;
        }

        bool BlockParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void BlockParser::init(BitcoinLikeBlockchainExplorer::Block *block) {
            _block = block;
        }

    }
}