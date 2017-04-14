/*
 *
 * OutputParser
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/04/2017.
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
#include "OutputParser.hpp"

namespace ledger {
    namespace core {

        bool OutputParser::Null() {
            return true;
        }

        bool OutputParser::Bool(bool b) {
            return true;
        }

        bool OutputParser::Int(int i) {
            return true;
        }

        bool OutputParser::Uint(unsigned int i) {
            return true;
        }

        bool OutputParser::Int64(int64_t i) {
            return true;
        }

        bool OutputParser::Uint64(uint64_t i) {
            return true;
        }

        bool OutputParser::Double(double d) {
            return true;
        }

        bool OutputParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            std::string number(str, length);
            BigInt value = BigInt::fromString(number);
            if (_lastKey == "output_index") {
                _output.index = value.toUint64();
            } else if (_lastKey == "value") {
                _output.value = value;
            }
            return true;
        }

        bool OutputParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            std::string value = std::string(str, length);
            if (_lastKey == "address") {
                _output.address = Option<std::string>(value);
            } else if (_lastKey == "script_hex") {
                _output.script = value;
            }
            return true;
        }

        bool OutputParser::StartObject() {
            return true;
        }

        bool OutputParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            _lastKey = std::string(str, length);
            return true;
        }

        bool OutputParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool OutputParser::StartArray() {
            return true;
        }

        bool OutputParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        Either<Exception, BitcoinLikeBlockchainExplorer::Output> OutputParser::build() {
            return Either<Exception, BitcoinLikeBlockchainExplorer::Output>(_output);
        }

        void OutputParser::reset() {
            _output = BitcoinLikeBlockchainExplorer::Output();
        }

        void OutputParser::attach(const std::shared_ptr<api::HttpUrlConnection> &connection) {
            _statusCode = connection->getStatusCode();
            _statusText = connection->getStatusText();
        }
    }
}