/*
 *
 * AbstractWebSocketNotificationParser
 *
 * Created by El Khalil Bellakrid on 29/11/2018.
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


#ifndef LEDGER_CORE_ABSTRACTWEBSOCKETNOTIFICATIONPARSER_H
#define LEDGER_CORE_ABSTRACTWEBSOCKETNOTIFICATIONPARSER_H

#define PROXY_PARSE_WS(method, ...)                                         \
 auto& currentObject = _currentObject;                                   \
 if (currentObject == "block") {                                         \
    return getBlockParser().method(__VA_ARGS__);                             \
 } else if (currentObject == "transaction") {                            \
    return getTransactionParser().method(__VA_ARGS__);                       \
 } else                                                                  \

namespace ledger {
    namespace core {
        template<typename BlockchainExplorerTransaction, typename BlockchainExplorerBlock, typename TransactionParser, typename BlockParser>
        class AbstractWebSocketNotificationParser {
        public:
            struct Result {
                BlockchainExplorerTransaction transaction;
                BlockchainExplorerBlock block;
                std::string type;
                std::string blockchain;
            };

            void init(Result *result) {
                _result = result;
            };

            bool Null() {
                PROXY_PARSE_WS(Null) {
                    return true;
                }
            };

            bool Bool(bool b) {
                PROXY_PARSE_WS(Bool, b) {
                    return true;
                }
            };

            bool Int(int i) {
                PROXY_PARSE_WS(Int, i) {
                    return true;
                }
            };

            bool Uint(unsigned i) {
                PROXY_PARSE_WS(Uint, i) {
                    return true;
                }
            };

            bool Int64(int64_t i) {
                PROXY_PARSE_WS(Int64, i) {
                    return true;
                }
            };

            bool Uint64(uint64_t i) {
                PROXY_PARSE_WS(Uint64, i) {
                    return true;
                }
            };

            bool Double(double d) {
                PROXY_PARSE_WS(Double, d) {
                    return true;
                }
            };

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length,
                                                        bool copy) {
                PROXY_PARSE_WS(RawNumber, str, length, copy) {
                    return true;
                }
            };

            bool
            String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_WS(String, str, length, copy) {
                    if (getLastKey() == "type") {
                        _result->type = std::string(str, length);
                    } else if (getLastKey() == "block_chain") {
                        _result->blockchain = std::string(str, length);
                    }
                    return true;
                }
            };

            bool StartObject() {
                {
                    _depth += 1;
                    if (_depth == 3) {
                        _currentObject = getLastKey();
                    }
                    auto& currentObject = _currentObject;

                    if (currentObject == "transaction") {
                        getTransactionParser().init(&_result->transaction);
                    } else if (currentObject == "block") {
                        getBlockParser().init(&_result->block);
                    }
                }
                PROXY_PARSE_WS(StartObject) {
                    return true;
                }
            };


            virtual bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_WS(Key, str, length, copy) {
                    return true;
                }
            };

            bool EndObject(rapidjson::SizeType memberCount) {
                _depth -= 1;
                if (_depth == 2)
                    _currentObject = "";
                PROXY_PARSE_WS(EndObject, memberCount) {
                    return true;
                }
            };

            bool StartArray() {
                PROXY_PARSE_WS(StartArray) {
                    return true;
                }
            };

            bool EndArray(rapidjson::SizeType elementCount) {
                PROXY_PARSE_WS(EndArray, elementCount) {
                    return false;
                }
            };

        protected:
            virtual TransactionParser &getTransactionParser() = 0;
            virtual BlockParser &getBlockParser() = 0;
            virtual std::string &getLastKey() = 0;
        private:
            Result* _result;
            int32_t  _depth = 0;
            std::string _currentObject = "";
        };
    }
}

#endif //LEDGER_CORE_ABSTRACTWEBSOCKETNOTIFICATIONPARSER_H
