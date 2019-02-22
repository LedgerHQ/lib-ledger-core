/*
 *
 * AbstractBlockchainExplorer
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


#ifndef LEDGER_CORE_ABSTRACTBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_ABSTRACTBLOCKCHAINEXPLORER_H

#include <string>
#include <chrono>
#include <vector>

#include <async/Future.hpp>
#include <collections/collections.hpp>
#include <utils/optional.hpp>
#include <utils/Option.hpp>
#include <wallet/common/Block.h>

namespace ledger {
    namespace core {
        template <typename Transaction>
        class AbstractBlockchainExplorer {
        public:
            struct TransactionsBulk {
                std::vector<Transaction> transactions;
                bool hasNext;
                std::string marker; //Needed for pagination for XRP: https://developers.ripple.com/markers-and-pagination.html
            };

            virtual Future<void *> startSession() = 0;
            virtual Future<Unit> killSession(void *session) = 0;
            virtual FuturePtr<TransactionsBulk> getTransactions(const std::vector<std::string>& addresses,
                                                                Option<std::string> fromBlockHash = Option<std::string>(),
                                                                Option<void*> session = Option<void *>()) = 0;
            virtual FuturePtr<Block> getCurrentBlock() const = 0;
            virtual Future<Bytes> getRawTransaction(const String& transactionHash) = 0;
            virtual FuturePtr<Transaction> getTransactionByHash(const String& transactionHash) const = 0;
            virtual Future<String> pushTransaction(const std::vector<uint8_t>& transaction) = 0;
            virtual Future<int64_t> getTimestamp() const = 0;
        };
    }
}


#endif //LEDGER_CORE_ABSTRACTBLOCKCHAINEXPLORER_H
