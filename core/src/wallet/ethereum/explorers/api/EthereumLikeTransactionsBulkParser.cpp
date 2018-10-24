/*
 *
 * EthereumLikeTransactionBulkParser
 *
 * Created by El Khalil Bellakrid on 29/07/2018.
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


#include "EthereumLikeTransactionsBulkParser.h"

#define PROXY_PARSE(method, ...)                                            \
    if (_depth > 0) {                                                       \
        return _transactionsParser.method(__VA_ARGS__);                     \
    } else {                                                                \
        return true;                                                        \
    }

namespace ledger {
    namespace core {

        bool EthereumLikeTransactionsBulkParser::Null() {
            PROXY_PARSE(Null)
        }

        bool EthereumLikeTransactionsBulkParser::Bool(bool b) {
            if (_lastKey == "truncated" && _depth == 0) {
                _bulk->hasNext = b;
            }
            PROXY_PARSE(Bool, b)
        }

        bool EthereumLikeTransactionsBulkParser::Int(int i) {
            PROXY_PARSE(Int, i)
        }

        bool EthereumLikeTransactionsBulkParser::Uint(unsigned i) {
            PROXY_PARSE(Uint, i)
        }

        bool EthereumLikeTransactionsBulkParser::Int64(int64_t i) {
            PROXY_PARSE(Int64, i)
        }

        bool EthereumLikeTransactionsBulkParser::Uint64(uint64_t i) {
            PROXY_PARSE(Uint64, i)
        }

        bool EthereumLikeTransactionsBulkParser::Double(double d) {
            PROXY_PARSE(Double, d)
        }

        bool EthereumLikeTransactionsBulkParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(RawNumber, str, length, copy)
        }

        bool EthereumLikeTransactionsBulkParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(String, str, length, copy)
        }

        bool EthereumLikeTransactionsBulkParser::StartObject() {
            PROXY_PARSE(StartObject)
        }

        bool EthereumLikeTransactionsBulkParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(Key, str, length, copy)
        }

        bool EthereumLikeTransactionsBulkParser::EndObject(rapidjson::SizeType memberCount) {
            PROXY_PARSE(EndObject, memberCount)
        }

        bool EthereumLikeTransactionsBulkParser::StartArray() {
            if (_depth >= 1 || _lastKey == "txs") {
                _depth += 1;
            }

            if (_depth == 1) {
                _transactionsParser.init(&_bulk->transactions);
            }

            PROXY_PARSE(StartArray)
        }

        bool EthereumLikeTransactionsBulkParser::EndArray(rapidjson::SizeType elementCount) {
            if (_depth > 0) {
                _depth -= 1;
            }

            PROXY_PARSE(EndArray, elementCount)
        }

        EthereumLikeTransactionsBulkParser::EthereumLikeTransactionsBulkParser(std::string& lastKey) : _lastKey(lastKey), _transactionsParser(lastKey) {
            _depth = 0;
        }

        void EthereumLikeTransactionsBulkParser::init(EthereumLikeBlockchainExplorer::TransactionsBulk *bulk) {
            _bulk = bulk;
        }
    }
}