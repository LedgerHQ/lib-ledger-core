/*
 *
 * AbstractTransactionsParser
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

#include <rapidjson/reader.h>
#include <net/HttpClient.hpp>
#include <vector>


#ifndef LEDGER_CORE_ABSTRACTTRANSACTIONSPARSER_H
#define LEDGER_CORE_ABSTRACTTRANSACTIONSPARSER_H

#define PROXY_PARSE_TX(method, ...)                                            \
    if (_arrayDepth > 0) {                                                  \
        return getTransactionParser().method(__VA_ARGS__);                      \
    } else {                                                                \
        return true;                                                        \
    }

namespace ledger {
    namespace core {
        template <typename BlockchainExplorerTransaction, typename TxParser>
        class AbstractTransactionsParser {

        public:
            bool Null() {
                PROXY_PARSE_TX(Null)
            };

            bool Bool(bool b) {
                PROXY_PARSE_TX(Bool, b)
            };

            bool Int(int i) {
                PROXY_PARSE_TX(Int, i)
            };

            bool Uint(unsigned i) {
                PROXY_PARSE_TX(Uint, i)
            };

            bool Int64(int64_t i) {
                PROXY_PARSE_TX(Int64, i)
            };

            bool Uint64(uint64_t i) {
                PROXY_PARSE_TX(Uint64, i)
            };

            bool Double(double d) {
                PROXY_PARSE_TX(Double, d)
            };

            bool
            RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_TX(RawNumber, str, length, copy)
            };

            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_TX(String, str, length, copy)
            };

            bool StartObject() {
                _objectDepth += 1;

                if (_arrayDepth == 1 && _objectDepth == 1) {
                    BlockchainExplorerTransaction transaction;
                    _transactions->push_back(transaction);
                    getTransactionParser().init(&_transactions->back());
                }

                PROXY_PARSE_TX(StartObject)
            };

            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_TX(Key, str, length, copy)
            };

            bool EndObject(rapidjson::SizeType memberCount) {
                if (_arrayDepth > 0) {
                    _objectDepth -= 1;
                    auto result =  getTransactionParser().EndObject(memberCount);
                    return result;
                } else {
                    return true;
                }
            };

            bool StartArray() {
                if (_arrayDepth > 0) {
                    _arrayDepth = _arrayDepth + 1;
                    return getTransactionParser().StartArray();
                } else {
                    _arrayDepth = _arrayDepth + 1;
                    return true;
                }
            };

            bool EndArray(rapidjson::SizeType elementCount) {
                _arrayDepth -= 1;
                PROXY_PARSE_TX(EndArray, elementCount)
                return true;
            };


            void init(
                    std::vector<BlockchainExplorerTransaction> *transactions) {
                _transactions = transactions;
            };

        protected:
            virtual TxParser &getTransactionParser() = 0;
            std::vector<BlockchainExplorerTransaction>* _transactions;
            uint32_t _arrayDepth;
            uint32_t _objectDepth;
        };
    }
}

#endif //LEDGER_CORE_ABSTRACTTRANSACTIONSPARSER_H
