/*
 *
 * WebSocketNotificationParser.h
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

#ifndef LEDGER_CORE_WEBSOCKETNOTIFICATIONPARSER_H
#define LEDGER_CORE_WEBSOCKETNOTIFICATIONPARSER_H

#include "../../../../collections/collections.hpp"
#include <cstdio>
#include <cstdint>
#include "../BitcoinLikeBlockchainExplorer.hpp"
#include "../../../../net/HttpClient.hpp"
#include "BlockParser.hpp"
#include <rapidjson/reader.h>
#include <stack>
#include "TransactionParser.hpp"
#include <wallet/common/explorers/api/AbstractWebSocketNotificationParser.h>

namespace ledger {
    namespace core {
        class WebSocketNotificationParser : public AbstractWebSocketNotificationParser<BitcoinLikeBlockchainExplorerTransaction, BitcoinLikeBlockchainExplorer::Block, TransactionParser, BlockParser> {
        public:


            explicit WebSocketNotificationParser(std::string& lastKey) : _lastKey(lastKey),
                                                                        _blockParser(lastKey),
                                                                        _transactionParser(lastKey) {

            }

            bool Key(const rapidjson::Reader::Ch* str, rapidjson::SizeType length, bool copy) override {
                _lastKey = std::string(str, length);
                return AbstractWebSocketNotificationParser<BitcoinLikeBlockchainExplorerTransaction,
                        BitcoinLikeBlockchainExplorer::Block,
                        TransactionParser,
                        BlockParser>::Key(str, length, copy);
            }

        protected:

            TransactionParser &getTransactionParser() override {
              return _transactionParser;
            };
            BlockParser &getBlockParser() override {
                return _blockParser;
            };
            std::string &getLastKey() override {
                return _lastKey;
            };

        private:
            std::string& _lastKey;
            BlockParser _blockParser;
            TransactionParser _transactionParser;
        };
    }
}


#endif //LEDGER_CORE_WEBSOCKETNOTIFICATIONPARSER_H
