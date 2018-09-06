/*
 *
 * TransactionsBulk
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/04/2017.
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
#include "TransactionsBulkParser.hpp"

#define PROXY_PARSE(method, ...)                                            \
    if (_depth > 0) {                                                       \
        return _transactionsParser.method(__VA_ARGS__);                     \
    } else {                                                                \
        return true;                                                        \
    }

namespace ledger {
    namespace core {

        bool TransactionsBulkParser::Null() {
            PROXY_PARSE(Null)
        }

        bool TransactionsBulkParser::Bool(bool b) {
            if (_lastKey == "truncated" && _depth == 0) {
                _bulk->hasNext = b;
            }
            PROXY_PARSE(Bool, b)
        }

        bool TransactionsBulkParser::Int(int i) {
            PROXY_PARSE(Int, i)
        }

        bool TransactionsBulkParser::Uint(unsigned i) {
            PROXY_PARSE(Uint, i)
        }

        bool TransactionsBulkParser::Int64(int64_t i) {
            PROXY_PARSE(Int64, i)
        }

        bool TransactionsBulkParser::Uint64(uint64_t i) {
            PROXY_PARSE(Uint64, i)
        }

        bool TransactionsBulkParser::Double(double d) {
            PROXY_PARSE(Double, d)
        }

        bool TransactionsBulkParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(RawNumber, str, length, copy)
        }

        bool TransactionsBulkParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(String, str, length, copy)
        }

        bool TransactionsBulkParser::StartObject() {
            PROXY_PARSE(StartObject)
        }

        bool TransactionsBulkParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(Key, str, length, copy)
        }

        bool TransactionsBulkParser::EndObject(rapidjson::SizeType memberCount) {
            PROXY_PARSE(EndObject, memberCount)
        }

        bool TransactionsBulkParser::StartArray() {
            if (_depth >= 1 || _lastKey == "txs") {
                _depth += 1;
            }

            if (_depth == 1) {
                _transactionsParser.init(&_bulk->transactions);
            }

            PROXY_PARSE(StartArray)
        }

        bool TransactionsBulkParser::EndArray(rapidjson::SizeType elementCount) {
            if (_depth > 0) {
                _depth -= 1;
            }

            PROXY_PARSE(EndArray, elementCount)
        }

        TransactionsBulkParser::TransactionsBulkParser(std::string& lastKey) : _lastKey(lastKey), _transactionsParser(lastKey) {
            _depth = 0;
        }

        void TransactionsBulkParser::init(BitcoinLikeBlockchainExplorer::TransactionsBulk *bulk) {
            _bulk = bulk;
        }
    }
}