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

#define PROXY_PARSE(method, ...)                                    \
 auto& currentObject = _hierarchy.top();                            \
 if (currentObject == "block") {                                    \
    return _blockParser.method(__VA_ARGS__);                        \
 } else if (currentObject == "inputs") {                            \
    return _blockParser.method(__VA_ARGS__);                        \
 } else if (currentObject == "outputs") {                           \
    return _blockParser.method(__VA_ARGS__);                        \
 } else                                                             \

namespace ledger {
    namespace core {

        void TransactionParser::attach(const std::shared_ptr<api::HttpUrlConnection>& connection) {
            _statusText = connection->getStatusText();
            _statusCode = (uint32_t) connection->getStatusCode();
        }

        Either<Exception, BitcoinLikeBlockchainExplorer::Transaction> TransactionParser::build() {
            if (_statusCode == 200) {
                return Either<Exception, BitcoinLikeBlockchainExplorer::Transaction>(buildTransaction());
            } else {
                return Either<Exception, BitcoinLikeBlockchainExplorer::Transaction>(buildException());
            }
        }

        Exception TransactionParser::buildException() {
            return Exception(api::ErrorCode::API_ERROR, _statusText);
        }

        BitcoinLikeBlockchainExplorer::Transaction TransactionParser::buildTransaction() {
            return _transaction;
        }

        bool TransactionParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            _lastKey = std::string(str, length);
            return true;
        }

        bool TransactionParser::StartObject() {
            _hierarchy.push(_lastKey);
            return true;
        }

        bool TransactionParser::EndObject(rapidjson::SizeType memberCount) {
            _hierarchy.pop();
            _blockParser.reset();
            return true;
        }

        bool TransactionParser::StartArray() {
            StartObject();
            return true;
        }

        bool TransactionParser::EndArray(rapidjson::SizeType elementCount) {
            EndObject(elementCount);
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
            PROXY_PARSE(Int, i) {
                return true;
            }
        }

        bool TransactionParser::Uint(unsigned i) {
            PROXY_PARSE(Uint, i) {
                return true;
            }
        }

        bool TransactionParser::Int64(int64_t i) {
            PROXY_PARSE(Int64, i) {
                return true;
            }
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
                return true;
            }
        }

        bool TransactionParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(String, str, length, copy) {
                std::string value(str, length);
                if (_lastKey == "hash") {
                    _transaction.hash = value;
                } else if (_lastKey == "received_at") {

                }
                return true;
            }
        }

    }
}