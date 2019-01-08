/*
 *
 * RippleLikeTransactionParser
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


#include <wallet/currencies.hpp>
#include "RippleLikeTransactionParser.h"

#define PROXY_PARSE(method, ...)                                    \
 auto& currentObject = _hierarchy.top();                            \
 if (currentObject == "block") {                                    \
    return _blockParser.method(__VA_ARGS__);                        \
 } else                                                             \

namespace ledger {
    namespace core {

        bool RippleLikeTransactionParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(Key, str, length, copy) {
                return true;
            }
        }

        bool RippleLikeTransactionParser::StartObject() {
            if (_arrayDepth == 0) {
                _hierarchy.push(_lastKey);
            }
            return true;
        }

        bool RippleLikeTransactionParser::EndObject(rapidjson::SizeType memberCount) {
            auto &currentObject = _hierarchy.top();
            if (_arrayDepth == 0) {
                _hierarchy.pop();
            }
            return true;
        }

        bool RippleLikeTransactionParser::StartArray() {
            if (_arrayDepth == 0) {
                _hierarchy.push(_lastKey);
            }
            _arrayDepth += 1;
            return true;
        }

        bool RippleLikeTransactionParser::EndArray(rapidjson::SizeType elementCount) {
            _arrayDepth -= 1;
            if (_arrayDepth == 0) {
                _hierarchy.pop();
            }
            return true;
        }

        bool RippleLikeTransactionParser::Null() {
            PROXY_PARSE(Null) {
                return true;
            }
        }

        bool RippleLikeTransactionParser::Bool(bool b) {
            PROXY_PARSE(Bool, b) {
                return true;
            }
        }

        bool RippleLikeTransactionParser::Int(int i) {
            return Uint64(i);
        }

        bool RippleLikeTransactionParser::Uint(unsigned i) {
            return Uint64(i);
        }

        bool RippleLikeTransactionParser::Int64(int64_t i) {
            return Uint64(i);
        }

        bool RippleLikeTransactionParser::Uint64(uint64_t i) {
            PROXY_PARSE(Uint64, i) {
                return true;
            }
        }

        bool RippleLikeTransactionParser::Double(double d) {
            PROXY_PARSE(Double, d) {
                return true;
            }
        }

        bool RippleLikeTransactionParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length,
                                                    bool copy) {
            PROXY_PARSE(RawNumber, str, length, copy) {

                std::string number(str, length);
                BigInt value = BigInt::fromString(number);
                if (_lastKey == "confirmations") {
                    _transaction->confirmations = value.toUint64();
                } else if (_lastKey == "ledger_index") {
                    RippleLikeBlockchainExplorer::Block block;
                    BigInt valueBigInt = BigInt::fromString(number);
                    block.height = valueBigInt.toUint64();
                    block.currencyName = currencies::RIPPLE.name;
                    _transaction->block = block;
                }

                return true;
            }
        }

        bool
        RippleLikeTransactionParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(String, str, length, copy) {
                std::string value(str, length);

                if (_lastKey == "hash") {
                    _transaction->hash = value;
                } else if (_lastKey == "date") {
                    auto pos = value.find('+');
                    if ( pos != std::string::npos && pos > 0) {
                        value = value.substr(0, pos);
                    }
                    auto posZ = value.find('Z');
                    if ( posZ == std::string::npos ) {
                        value = value + "Z";
                    }
                    auto date = DateUtils::fromJSON(value);
                    _transaction->receivedAt = date;
                    if (_transaction->block.hasValue()) {
                        _transaction->block.getValue().time = date;
                    }
                } else if (_lastKey == "Account" && currentObject == "tx") {
                    _transaction->sender = value;
                } else if (_lastKey == "Destination") {
                    _transaction->receiver = value;
                } else if (_lastKey == "Amount") {
                    BigInt valueBigInt = BigInt::fromString(value);
                    _transaction->value = valueBigInt;
                } else if (_lastKey == "Fee") {
                    BigInt valueBigInt = BigInt::fromString(value);
                    _transaction->fees = value;
                }
                return true;
            }
        }

        RippleLikeTransactionParser::RippleLikeTransactionParser(std::string &lastKey) :
                _lastKey(lastKey), _blockParser(lastKey) {
            _arrayDepth = 0;
        }

        void RippleLikeTransactionParser::init(RippleLikeBlockchainExplorerTransaction *transaction) {
            _transaction = transaction;
        }

    }
}