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
#ifndef LEDGER_CORE_TRANSACTIONSPARSER_HPP
#define LEDGER_CORE_TRANSACTIONSPARSER_HPP

#include "../BitcoinLikeBlockchainExplorer.hpp"
#include "TransactionParser.hpp"

#include <wallet/common/explorers/api/AbstractTransactionsParser.h>

namespace ledger {
    namespace core {
        class TransactionsParser : public AbstractTransactionsParser<BitcoinLikeBlockchainExplorerTransaction, TransactionParser> {
          public:
            TransactionsParser(std::string &lastKey) : _transactionParser(lastKey) {
                _arrayDepth = 0;
                _objectDepth = 0;
            }

            bool StartObject() {
                _objectDepth += 1;
                // In v2 /transactions/${hash} endpoint returns an array of tx object => _arrayDepth == 0
                // In v3 /transactions/${hash} endpoint returns a tx object => _arrayDepth == 1
                if ((_arrayDepth == 1 || _arrayDepth == 0) && _objectDepth == 1) {
                    BitcoinLikeBlockchainExplorerTransaction transaction;
                    _transactions->push_back(transaction);
                    getTransactionParser().init(&_transactions->back());
                }

                PROXY_PARSE_TX(StartObject)
            };

          protected:
            TransactionParser &getTransactionParser() override {
                return _transactionParser;
            }

          private:
            TransactionParser _transactionParser;
        };
    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_TRANSACTIONSPARSER_HPP
