/*
 *
 * InputParser
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/04/2017.
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
#include "InputParser.hpp"
#include "../../../../utils/DateParser.hpp"

namespace ledger {
    namespace core {

        bool InputParser::Null() {
            return true;
        }

        bool InputParser::Bool(bool b) {
            return true;
        }

        bool InputParser::Int(int i) {
            return true;
        }

        bool InputParser::Uint(unsigned i) {
            return true;
        }

        bool InputParser::Int64(int64_t i) {
            return true;
        }

        bool InputParser::Uint64(uint64_t i) {
            return true;
        }

        bool InputParser::Double(double d) {
            return true;
        }

        bool InputParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            std::string number(str, length);
            BigInt value = BigInt::fromString(number);
            if (_lastKey == "input_index") {
                _input.index = value.toUint64();
            } else if (_lastKey == "value") {
                _input.value = Option<BigInt>(value);
            } else if (_lastKey == "output_index") {
                _input.previousTxOutputIndex = value.toUint64();
            }
            return true;
        }

        bool InputParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            std::string value = std::string(str, length);
            if (_lastKey == "output_hash") {
                _input.previousTxHash = value;
            } else if (_lastKey == "address") {
                _input.address = Option<std::string>(value);
            } else if (_lastKey == "script_signature") {
                _input.signatureScript = Option<std::string>(value);
            } else if (_lastKey == "coinbase") {
                _input.coinbase = Option<std::string>(value);
            }
            return true;
        }

        bool InputParser::StartObject() {
            return true;
        }

        bool InputParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            _lastKey = std::string(str, length);
            return true;
        }

        bool InputParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool InputParser::StartArray() {
            return true;
        }

        bool InputParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        BitcoinLikeBlockchainExplorer::Input InputParser::build() {
            return _input;
        }

        void InputParser::reset() {
            _input = BitcoinLikeBlockchainExplorer::Input();
        }
    }
}