/*
 *
 * TransactionsParser
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
#include "TransactionsParser.hpp"

#define PROXY_PARSE(method, ...)                                            \
    if (_arrayDepth > 0) {                                                  \
        return _transactionParser.method(__VA_ARGS__);                      \
    } else {                                                                \
        return true;                                                        \
    }


bool ledger::core::TransactionsParser::Null() {
    PROXY_PARSE(Null)
}

bool ledger::core::TransactionsParser::Bool(bool b) {
    PROXY_PARSE(Bool, b)
}

bool ledger::core::TransactionsParser::Int(int i) {
    PROXY_PARSE(Int, i)
}

bool ledger::core::TransactionsParser::Uint(unsigned i) {
    PROXY_PARSE(Uint, i)
}

bool ledger::core::TransactionsParser::Int64(int64_t i) {
    PROXY_PARSE(Int64, i)
}

bool ledger::core::TransactionsParser::Uint64(uint64_t i) {
    PROXY_PARSE(Uint64, i)
}

bool ledger::core::TransactionsParser::Double(double d) {
    PROXY_PARSE(Double, d)
}

bool
ledger::core::TransactionsParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
    PROXY_PARSE(RawNumber, str, length, copy)
}

bool ledger::core::TransactionsParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
    PROXY_PARSE(String, str, length, copy)
}

bool ledger::core::TransactionsParser::StartObject() {
    _objectDepth += 1;

    if (_arrayDepth == 1 && _objectDepth == 1) {
        BitcoinLikeBlockchainExplorerTransaction transaction;
        _transactions->push_back(transaction);
        _transactionParser.init(&_transactions->back());
    }

    PROXY_PARSE(StartObject)
}

bool ledger::core::TransactionsParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
    PROXY_PARSE(Key, str, length, copy)
}

bool ledger::core::TransactionsParser::EndObject(rapidjson::SizeType memberCount) {
    if (_arrayDepth > 0) {
        _objectDepth -= 1;
        auto result =  _transactionParser.EndObject(memberCount);
        return result;
    } else {
        return true;
    }
}

bool ledger::core::TransactionsParser::StartArray() {
    if (_arrayDepth > 0) {
        _arrayDepth = _arrayDepth + 1;
        return _transactionParser.StartArray();
    } else {
        _arrayDepth = _arrayDepth + 1;
        return true;
    }
}

bool ledger::core::TransactionsParser::EndArray(rapidjson::SizeType elementCount) {
    _arrayDepth -= 1;
    PROXY_PARSE(EndArray, elementCount)
    return true;
}

ledger::core::TransactionsParser::TransactionsParser(std::string& lastKey) : _lastKey(lastKey), _transactionParser(lastKey) {
    _arrayDepth = 0;
    _objectDepth = 0;
}

void ledger::core::TransactionsParser::init(
std::vector<ledger::core::BitcoinLikeBlockchainExplorerTransaction> *transactions) {
    _transactions = transactions;
}
