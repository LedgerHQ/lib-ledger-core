/*
 *
 * EthereumLikeTransactionsBulkParser
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


#ifndef LEDGER_CORE_ETHEREUMLIKETRANSACTIONSBULKPARSER_H
#define LEDGER_CORE_ETHEREUMLIKETRANSACTIONSBULKPARSER_H

#include "../EthereumLikeBlockchainExplorer.h"
#include <wallet/ethereum/explorers/api/EthereumLikeTransactionsParser.h>
#include <wallet/common/explorers/api/AbstractTransactionsBulkParser.h>
namespace ledger {
    namespace core {
        class EthereumLikeBlockchainExplorer;
        class EthereumLikeTransactionsBulkParser : public AbstractTransactionsBulkParser<EthereumLikeBlockchainExplorer::TransactionsBulk, EthereumLikeTransactionsParser> {
        public:
            EthereumLikeTransactionsBulkParser(std::string& lastKey) : _lastKey(lastKey),
                                                                       _transactionsParser(lastKey)
            {
                _depth = 0;
            };

        protected:
            EthereumLikeTransactionsParser &getTransactionsParser() override {
                return _transactionsParser;
            }

            std::string &getLastKey() override {
                return _lastKey;
            }
        private:
            EthereumLikeTransactionsParser _transactionsParser;
            std::string& _lastKey;
        };
    }
}


#endif //LEDGER_CORE_ETHEREUMLIKETRANSACTIONSBULKPARSER_H
