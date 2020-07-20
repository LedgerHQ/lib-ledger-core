/*
 * AlgorandOperationDatabaseHelper
 *
 * Created by Rémi Barjon on 20/07/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include <algorand/database/AlgorandOperationDatabaseHelper.hpp>

#include <soci.h>

namespace ledger {
namespace core {
namespace algorand {

    bool OperationDatabaseHelper::putAlgorandOperation(
            soci::session& sql,
            const std::string& txUid,
            const Operation& operation)
    {
        const auto newOperation = ledger::core::OperationDatabaseHelper::putOperation(sql, operation);
        const auto opUid = operation.uid;
        const auto txHash = operation.getTransaction()->getId();

        if (newOperation) {
            sql << "INSERT INTO algorand_operations VALUES(:uid, :tx_uid, :tx_hash)",
                soci::use(opUid),
                soci::use(txUid),
                soci::use(txHash);
        } else {
            sql << "UPDATE algorand_operations SET tx_uid = :tx_uid, tx_hash = :tx_hash WHERE uid = :op_uid",
                soci::use(txUid),
                soci::use(txHash),
                soci::use(opUid);
        }
        return newOperation;
    }

} // namespace algorand
} // namespace core
} // namespace ledger

