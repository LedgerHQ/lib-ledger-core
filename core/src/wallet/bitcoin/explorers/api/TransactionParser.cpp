/*
 *
 * TransactionParser
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
#include "TransactionParser.hpp"
#include "utils/DateUtils.hpp"

#define PROXY_PARSE(method, ...)                                    \
 auto& currentObject = _hierarchy.top();                            \
 if (currentObject == "block") {                                    \
    return _blockParser.method(__VA_ARGS__);                        \
 } else if (currentObject == "inputs") {                            \
    return _inputParser.method(__VA_ARGS__);                        \
 } else if (currentObject == "outputs") {                           \
    return _outputParser.method(__VA_ARGS__);                        \
 } else                                                             \

namespace ledger {
    namespace core {

        bool TransactionParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            _lastKey = std::string(str, length);
            PROXY_PARSE(Key, str, length, copy) {
                return true;
            }
        }

        bool TransactionParser::StartObject() {
            if (_arrayDepth == 0) {
                _hierarchy.push(_lastKey);
            }

            auto& currentObject = _hierarchy.top();

            if (currentObject == "inputs") {
                BitcoinLikeBlockchainExplorer::Input input;
                _transaction->inputs.push_back(input);
                _inputParser.init(&_transaction->inputs.back());
            } else if (currentObject == "outputs") {
                BitcoinLikeBlockchainExplorer::Output output;
                _transaction->outputs.push_back(output);
                _outputParser.init(&_transaction->outputs.back());
            } else if (currentObject == "block") {
                BitcoinLikeBlockchainExplorer::Block block;
                _transaction->block = Option<BitcoinLikeBlockchainExplorer::Block>(block);
                _blockParser.init(&_transaction->block.getValue());
            }

            return true;
        }

        bool TransactionParser::EndObject(rapidjson::SizeType memberCount) {
            auto& currentObject = _hierarchy.top();

            if (_arrayDepth == 0) {
                _hierarchy.pop();
            }
            return true;
        }

        bool TransactionParser::StartArray() {
            if (_arrayDepth == 0) {
                _hierarchy.push(_lastKey);
            }
            _arrayDepth += 1;
            return true;
        }

        bool TransactionParser::EndArray(rapidjson::SizeType elementCount) {
            _arrayDepth -= 1;
            if (_arrayDepth == 0) {
                _hierarchy.pop();
            }
            return true;
        }

        bool TransactionParser::Null() {
            PROXY_PARSE(Null) {
                return true;
            }
        }

        bool TransactionParser::Bool(bool b) {
            PROXY_PARSE(Bool, b) {
                return true;
            }
        }

        bool TransactionParser::Int(int i) {
            return Uint64(i);
        }

        bool TransactionParser::Uint(unsigned i) {
            return Uint64(i);
        }

        bool TransactionParser::Int64(int64_t i) {
            return Uint64(i);
        }

        bool TransactionParser::Uint64(uint64_t i) {
            PROXY_PARSE(Uint64, i) {
                return true;
            }
        }

        bool TransactionParser::Double(double d) {
            PROXY_PARSE(Double, d) {
                return true;
            }
        }

        bool TransactionParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(RawNumber, str, length, copy) {
                std::string number(str, length);
                BigInt value = BigInt::fromString(number);
                if (_lastKey == "lock_time") {
                    _transaction->lockTime = value.toUint64();
                } else if (_lastKey == "fees") {
                    _transaction->fees = Option<BigInt>(value);
                } else if (_lastKey == "confirmations") {
                    _transaction->confirmations = value.toUint64();
                }
                return true;
            }
        }

        bool TransactionParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(String, str, length, copy) {
                std::string value(str, length);
                if (_lastKey == "hash") {
                    _transaction->hash = value;
                } else if (_lastKey == "received_at") {
                    _transaction->receivedAt = DateUtils::fromJSON(value);
                }
                return true;
            }
        }

        TransactionParser::TransactionParser(std::string& lastKey) :
            _lastKey(lastKey), _blockParser(lastKey), _inputParser(lastKey), _outputParser(lastKey)
        {
            _arrayDepth = 0;
        }

        void TransactionParser::init(BitcoinLikeBlockchainExplorer::Transaction *transaction) {
            _transaction = transaction;
        }

    }
}