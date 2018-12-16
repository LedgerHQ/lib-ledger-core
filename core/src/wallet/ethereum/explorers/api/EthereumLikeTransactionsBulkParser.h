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

namespace ledger {
    namespace core {
        class EthereumLikeBlockchainExplorer;
        class EthereumLikeTransactionsBulkParser {
        public:
            typedef EthereumLikeBlockchainExplorer::TransactionsBulk Result;
            EthereumLikeTransactionsBulkParser(std::string& lastKey);
            void init(EthereumLikeBlockchainExplorer::TransactionsBulk* bulk);
            bool Null();
            bool Bool(bool b);
            bool Int(int i);
            bool Uint(unsigned i);
            bool Int64(int64_t i);
            bool Uint64(uint64_t i);
            bool Double(double d);
            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy);
            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy);
            bool StartObject();
            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy);
            bool EndObject(rapidjson::SizeType memberCount);
            bool StartArray();
            bool EndArray(rapidjson::SizeType elementCount);

        private:
            EthereumLikeBlockchainExplorer::TransactionsBulk* _bulk;
            EthereumLikeTransactionsParser _transactionsParser;
            std::string& _lastKey;
            int _depth;
        };
    }
}


#endif //LEDGER_CORE_ETHEREUMLIKETRANSACTIONSBULKPARSER_H
