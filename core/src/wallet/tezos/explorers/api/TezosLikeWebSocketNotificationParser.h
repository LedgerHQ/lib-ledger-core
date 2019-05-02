/*
 *
 * TezosLikeWebSocketNotificationParser
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#ifndef LEDGER_CORE_TEZOSLIKEWEBSOCKETNOTIFICATIONPARSER_H
#define LEDGER_CORE_TEZOSLIKEWEBSOCKETNOTIFICATIONPARSER_H

#include <cstdio>
#include <cstdint>
#include <rapidjson/reader.h>
#include <stack>
#include <net/HttpClient.hpp>
#include <collections/collections.hpp>
#include "../TezosLikeBlockchainExplorer.h"
#include "TezosLikeBlockParser.h"
#include "TezosLikeTransactionParser.h"
#include <wallet/common/explorers/api/AbstractWebSocketNotificationParser.h>


#define PROXY_PARSE_TEZOS_WS(method, ...)              \
 auto& currentObject = _currentObject;                  \
 if (currentObject == "transaction") {                  \
    return getTransactionParser().method(__VA_ARGS__);  \
 } else                                                 \

namespace ledger {
    namespace core {
        class TezosLikeWebSocketNotificationParser
                : public AbstractWebSocketNotificationParser<TezosLikeBlockchainExplorerTransaction, TezosLikeBlockchainExplorer::Block, TezosLikeTransactionParser, TezosLikeBlockParser> {
        public:


            explicit TezosLikeWebSocketNotificationParser(std::string &lastKey) : _lastKey(lastKey),
                                                                                  _blockParser(lastKey),
                                                                                  _transactionParser(lastKey) {

            }

            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) override {
                _lastKey = std::string(str, length);
                return AbstractWebSocketNotificationParser<TezosLikeBlockchainExplorerTransaction,
                        TezosLikeBlockchainExplorer::Block,
                        TezosLikeTransactionParser,
                        TezosLikeBlockParser>::Key(str, length, copy);
            }

            bool StartObject() {
                {
                    _depth += 1;
                    auto lastKey = getLastKey();
                    if (lastKey == "transaction") {
                        _currentObject = lastKey;
                        getTransactionParser().init(&_result->transaction);
                    }
                }
                PROXY_PARSE_TEZOS_WS(StartObject) {
                    return true;
                }
            };

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length,
                           bool copy) {
                PROXY_PARSE_TEZOS_WS(String, str, length, copy) {
                    auto value = std::string(str, length);
                    if (getLastKey() == "block_height") {
                        BigInt bigIntValue = BigInt::fromString(value);
                        _result->block.height = bigIntValue.toUint64();
                    }
                }
                return true;
            };

            bool
            String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {

                auto value = std::string(str, length);
                if (getLastKey() == "type") {
                    _result->type = value;
                }

                PROXY_PARSE_TEZOS_WS(String, str, length, copy) {
                    if (getLastKey() == "block_hash") {
                        _result->block.hash = value;
                    }
                    return true;
                }
            };

            bool EndObject(rapidjson::SizeType memberCount) {
                _depth -= 1;
                if (_depth == 1)
                    _currentObject = "";
                PROXY_PARSE_WS(EndObject, memberCount) {
                    return true;
                }
            };

        protected:

            TezosLikeTransactionParser &getTransactionParser() override {
                return _transactionParser;
            };

            TezosLikeBlockParser &getBlockParser() override {
                return _blockParser;
            };

            std::string &getLastKey() override {
                return _lastKey;
            };

        private:
            std::string &_lastKey;
            TezosLikeBlockParser _blockParser;
            TezosLikeTransactionParser _transactionParser;
        };
    }
}

#endif //LEDGER_CORE_TEZOSLIKEWEBSOCKETNOTIFICATIONPARSER_H
