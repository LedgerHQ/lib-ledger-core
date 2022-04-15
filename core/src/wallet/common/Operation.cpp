/*
 *
 * Operation
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/06/2017.
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
#include "Operation.h"
#include "database/OperationDatabaseHelper.h"
#include <utils/Exception.hpp>
#include <fmt/format.h>
namespace ledger {
    namespace core {

        void Operation::refreshUid(const std::string &additional) {
            std::string txId;
            if (bitcoinTransaction.nonEmpty()) {
                txId = computeTransactionId(bitcoinTransaction.getValue().hash);
            }
            else if (cosmosTransaction.nonEmpty()) {
                txId = computeTransactionId(cosmosTransaction.getValue().tx.hash, additional);
            }
            else if (ethereumTransaction.nonEmpty()) {
                txId = computeTransactionId(ethereumTransaction.getValue().hash);
            }
            else if (rippleTransaction.nonEmpty()) {
                txId = computeTransactionId(rippleTransaction.getValue().hash);
            }
            else if (tezosTransaction.nonEmpty()) {
                const auto& tx = tezosTransaction.getValue();
                std::string txIdBase = fmt::format("{}", tx.counter);
                txId = computeTransactionId(txIdBase, tx.type, additional);
            } else if (stellarOperation.nonEmpty()) {
                txId = computeTransactionId(stellarOperation.getValue().operation.transactionHash);
            } else {
                throw Exception(api::ErrorCode::RUNTIME_ERROR, "Cannot refresh uid of an incomplete operation.");
            }

            uid = OperationDatabaseHelper::createUid(accountUid, txId, type);
        }

        std::string Operation::computeTransactionId(const std::string& txHash, const std::string& additional){
            return additional.empty() ? txHash : fmt::format("{}+{}", txHash, additional);
        }

    }
}
