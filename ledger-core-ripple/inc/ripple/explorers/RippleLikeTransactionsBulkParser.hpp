/*
 *
 * RippleLikeTransactionsBulkParser
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

#pragma once

#include <core/explorers/AbstractTransactionsBulkParser.hpp>

#include <ripple/explorers/RippleLikeBlockchainExplorer.hpp>
#include <ripple/explorers/RippleLikeTransactionsParser.hpp>

namespace ledger {
    namespace core {
        class RippleLikeBlockchainExplorer;

        class RippleLikeTransactionsBulkParser
                : public AbstractTransactionsBulkParser<RippleLikeBlockchainExplorer::TransactionsBulk, RippleLikeTransactionsParser> {
        public:
            RippleLikeTransactionsBulkParser(std::string &lastKey):
                _lastKey(lastKey),
                _transactionsParser(lastKey),
                _inPaginationMarker(false) {
                _depth = 0;
            }

            bool StartArray() {
                if (_depth >= 1 || getLastKey() == "transactions") {
                    _depth += 1;
                }

                if (_depth == 1) {
                    getTransactionsParser().init(&_bulk->transactions);
                }

                PROXY_PARSE_TXS(StartArray)
            };

            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                PROXY_PARSE_TXS(String, str, length, copy)
            }

            bool StartObject() {
                if (_lastKey == "marker") {
                    _inPaginationMarker = true;
                }

                PROXY_PARSE_TXS(StartObject)
            }

            bool EndObject(rapidjson::SizeType memberCount) {
                if (_inPaginationMarker) {
                    _bulk->paginationMarker = fmt::format("{}-{}",
                            _paginationMarkerLedger,
                            _paginationMarkerSeq
                    );
                    _paginationMarkerLedger = "";
                    _paginationMarkerSeq = "";
                    _inPaginationMarker = false;
                }

                PROXY_PARSE_TXS(EndObject, memberCount)
            }

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                if (_inPaginationMarker) {
                    if (_lastKey == "ledger") {
                        _paginationMarkerLedger = std::string(str, length);
                    } else if (_lastKey == "seq") {
                        _paginationMarkerSeq = std::string(str, length);
                    }
                }

                PROXY_PARSE_TXS(RawNumber, str, length, copy)
            }

            bool Bool(bool b) {
                PROXY_PARSE_TXS(Bool, b)
            }


        protected:
            RippleLikeTransactionsParser &getTransactionsParser() override {
                return _transactionsParser;
            }

            std::string &getLastKey() override {
                return _lastKey;
            }

        private:
            RippleLikeTransactionsParser _transactionsParser;
            std::string &_lastKey;
            bool _inPaginationMarker;
            std::string _paginationMarkerLedger;
            std::string _paginationMarkerSeq;
        };
    }
}
