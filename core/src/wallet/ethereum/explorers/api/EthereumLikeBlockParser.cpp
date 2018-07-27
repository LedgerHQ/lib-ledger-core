/*
 *
 * EthereumLikeBlockParser
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
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

#include "EthereumLikeBlockParser.hpp"
#include "utils/DateUtils.hpp"

namespace ledger {
    namespace core {

        bool EthereumLikeBlockParser::Null() {
            return true;
        }

        bool EthereumLikeBlockParser::Bool(bool b) {
            return true;
        }

        bool EthereumLikeBlockParser::Int(int i) {
            return true;
        }

        bool EthereumLikeBlockParser::Uint(unsigned i) {
            return true;
        }

        bool EthereumLikeBlockParser::Int64(int64_t i) {
            return true;
        }

        bool EthereumLikeBlockParser::Uint64(uint64_t i) {
            return true;
        }

        bool EthereumLikeBlockParser::Double(double d) {
            return true;
        }

        bool EthereumLikeBlockParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_lastKey == "height") {
                std::string number(str, length);
                BigInt value = BigInt::fromString(number);
                _block->height = value.toUint64();
            }
            return true;
        }

        bool EthereumLikeBlockParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            std::string value = std::string(str, length);
            if (_lastKey == "hash") {
                _block->hash = value;
            } else if (_lastKey == "time") {
                _block->time = DateUtils::fromJSON(value);
            }
            return true;
        }

        bool EthereumLikeBlockParser::StartObject() {
            return true;
        }

        bool EthereumLikeBlockParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool EthereumLikeBlockParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool EthereumLikeBlockParser::StartArray() {
            return true;
        }

        bool EthereumLikeBlockParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void EthereumLikeBlockParser::init(EthereumLikeBlockchainExplorer::Block *block) {
            _block = block;
        }

    }
}