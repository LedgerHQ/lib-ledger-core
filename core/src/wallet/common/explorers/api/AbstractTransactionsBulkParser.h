/*
 *
 * AbstractTransactionsBulkParser
 *
 * Created by El Khalil Bellakrid on 17/11/2018.
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


#ifndef LEDGER_CORE_ABSTRACTTRANSACTIONSBULKPARSER_H
#define LEDGER_CORE_ABSTRACTTRANSACTIONSBULKPARSER_H

#define PROXY_PARSE_TXS(method, ...)                                            \
    if (_depth > 0) {                                                       \
        return getTransactionsParser().method(__VA_ARGS__);                     \
    } else {                                                                \
        return true;                                                        \
    }

namespace ledger {
    namespace core {
        template <typename BlockchainExplorerTransactionsBulk, typename TxsParser>
        class AbstractTransactionsBulkParser {
        public:

            bool Null() {
                PROXY_PARSE_TXS(Null)
            }

            bool Bool(bool b) {
                if (getLastKey() == "truncated" && _depth == 0) {
                    _bulk->hasNext = b;
                }
                PROXY_PARSE_TXS(Bool, b)
            }

            bool Int(int i) {
                PROXY_PARSE_TXS(Int, i)
            }

            bool Uint(unsigned i) {
                PROXY_PARSE_TXS(Uint, i)
            }

            bool Int64(int64_t i) {
                PROXY_PARSE_TXS(Int64, i)
            }

            bool Uint64(uint64_t i) {
                PROXY_PARSE_TXS(Uint64, i)
            }

            bool Double(double d) {
                PROXY_PARSE_TXS(Double, d)
            }

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_TXS(RawNumber, str, length, copy)
            }

            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_TXS(String, str, length, copy)
            }

            bool StartObject() {
                PROXY_PARSE_TXS(StartObject)
            }

            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_TXS(Key, str, length, copy)
            }

            bool EndObject(rapidjson::SizeType memberCount) {
                PROXY_PARSE_TXS(EndObject, memberCount)
            }

            bool StartArray() {
                if (_depth >= 1 || getLastKey() == "txs") {
                    _depth += 1;
                }

                if (_depth == 1) {
                    getTransactionsParser().init(&_bulk->transactions);
                }

                PROXY_PARSE_TXS(StartArray)
            }

            bool EndArray(rapidjson::SizeType elementCount) {
                if (_depth > 0) {
                    _depth -= 1;
                }

                PROXY_PARSE_TXS(EndArray, elementCount)
            }

            void init(BlockchainExplorerTransactionsBulk *bulk) {
                _bulk = bulk;
            }

        protected:
            virtual TxsParser &getTransactionsParser() = 0;
            virtual std::string &getLastKey() = 0;
            int _depth;
            BlockchainExplorerTransactionsBulk* _bulk;
        };

    }
}

#endif //LEDGER_CORE_ABSTRACTTRANSACTIONSBULKPARSER_H
