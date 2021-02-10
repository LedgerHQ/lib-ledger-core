/*
 *
 * CosmosLikeOperationDatabaseHelper
 *
 * Created by Hakim Aammar on 11/02/2020.
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

#ifndef LEDGER_CORE_COSMOSLIKEOPERATIONDATABASEHELPER_H
#define LEDGER_CORE_COSMOSLIKEOPERATIONDATABASEHELPER_H

#include <string>

#include <api/BigInt.hpp>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <database/soci-option.h>
#include <soci.h>
#include <wallet/common/database/OperationDatabaseHelper.h>

namespace ledger {
namespace core {
class CosmosLikeOperationDatabaseHelper : public OperationDatabaseHelper {
   public:
    static std::size_t queryOperations(
        soci::session &sql,
        const std::string &accountUid,
        std::vector<Operation> &operations,
        std::function<bool(const std::string &address)> filter)
    {
        using namespace soci;
        rowset<row> rows =
            (sql.prepare << "SELECT op.amount, op.fees, op.type, op.date, op.senders, op.recipients"
                            " FROM operations AS op "
                            " WHERE op.account_uid = :uid ORDER BY op.date",
             use(accountUid));
        const auto COL_AMT = 0;
        const auto COL_FEES = 1;
        const auto COL_TYPE = 2;
        const auto COL_DATE = 3;
        const auto COL_SEND = 4;
        const auto COL_RECV = 5;

        auto filterList = [&](const std::vector<std::string> &list) -> bool {
            for (auto &elem : list) {
                if (filter(elem)) {
                    return true;
                }
            }
            return false;
        };

        std::size_t c = 0;
        for (auto &row : rows) {
            auto type = api::from_string<api::OperationType>(row.get<std::string>(COL_TYPE));
            auto senders = strings::split(row.get<std::string>(COL_SEND), ",");
            auto recipients = strings::split(row.get<std::string>(COL_RECV), ",");
            if ((type == api::OperationType::SEND && row.get_indicator(COL_SEND) != i_null &&
                 filterList(senders)) ||
                (type == api::OperationType::RECEIVE && row.get_indicator(COL_RECV) != i_null &&
                 filterList(recipients)) ||
                (type == api::OperationType::NONE && row.get_indicator(COL_SEND) != i_null &&
                 filterList(senders))) {
                Operation operation;

                operation.amount = BigInt::fromHex(row.get<std::string>(COL_AMT));
                operation.fees = BigInt::fromHex(row.get<std::string>(COL_FEES));
                operation.type = type;
                operation.date = DateUtils::fromJSON(row.get<std::string>(COL_DATE));

                operations.push_back(operation);

                c += 1;
            }
        }
        return c;
    }

};
}  // namespace core
}  // namespace ledger

#endif  // LEDGER_CORE_COSMOSLIKEOPERATIONDATABASEHELPER_H
