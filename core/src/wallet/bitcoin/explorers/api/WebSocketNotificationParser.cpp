/*
 *
 * WebSocketNotificationParser.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/10/2017.
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

#include "WebSocketNotificationParser.h"


#define PROXY_PARSE(method, ...)                                         \
 auto& currentObject = _currentObject;                                   \
 if (currentObject == "block") {                                         \
    return _blockParser.method(__VA_ARGS__);                             \
 } else if (currentObject == "transaction") {                            \
    return _transactionParser.method(__VA_ARGS__);                       \
 } else                                                                  \


namespace ledger {
    namespace core {
        namespace bitcoin {
            WebSocketNotificationParser::WebSocketNotificationParser(std::string &lastKey) :
                _lastKey(lastKey), _blockParser(lastKey), _transactionParser(lastKey), _depth(0) {

            }

            void WebSocketNotificationParser::init(WebSocketNotificationParser::Result *result) {
                _result = result;
            }

            bool WebSocketNotificationParser::Null() {
                PROXY_PARSE(Null) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::Bool(bool b) {
                PROXY_PARSE(Bool, b) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::Int(int i) {
                PROXY_PARSE(Int, i) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::Uint(unsigned i) {
                PROXY_PARSE(Uint, i) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::Int64(int64_t i) {
                PROXY_PARSE(Int64, i) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::Uint64(uint64_t i) {
                PROXY_PARSE(Uint64, i) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::Double(double d) {
                PROXY_PARSE(Double, d) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length,
                bool copy) {
                PROXY_PARSE(RawNumber, str, length, copy) {
                    return true;
                }
            }

            bool
                WebSocketNotificationParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE(String, str, length, copy) {
                    if (_lastKey == "type") {
                        _result->type = std::string(str, length);
                    }
                    else if (_lastKey == "block_chain") {
                        _result->blockchain = std::string(str, length);
                    }
                    return true;
                }
            }

            bool WebSocketNotificationParser::StartObject() {
                {
                    _depth += 1;
                    if (_depth == 3) {
                        _currentObject = _lastKey;
                    }
                    auto& currentObject = _currentObject;

                    if (currentObject == "transaction") {
                        _transactionParser.init(&_result->transaction);
                    }
                    else if (currentObject == "block") {
                        _blockParser.init(&_result->block);
                    }
                }
                PROXY_PARSE(StartObject) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                _lastKey = std::string(str, length);
                PROXY_PARSE(Key, str, length, copy) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::EndObject(rapidjson::SizeType memberCount) {
                _depth -= 1;
                if (_depth == 2)
                    _currentObject = "";
                PROXY_PARSE(EndObject, memberCount) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::StartArray() {
                PROXY_PARSE(StartArray) {
                    return true;
                }
            }

            bool WebSocketNotificationParser::EndArray(rapidjson::SizeType elementCount) {
                PROXY_PARSE(EndArray, elementCount) {
                    return false;
                }
            }
        }
    }
}