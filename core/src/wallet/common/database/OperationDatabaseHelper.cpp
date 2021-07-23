/*
 *
 * OperationDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 31/05/2017.
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
#include "OperationDatabaseHelper.h"
#include "BlockDatabaseHelper.h"
#include <crypto/SHA256.hpp>
#include <api/Amount.hpp>
#include <api/BigInt.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <wallet/ethereum/database/EthereumLikeTransactionDatabaseHelper.h>
#include <wallet/ripple/database/RippleLikeTransactionDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>
#include <bytes/serialization.hpp>
#include <collections/strings.hpp>
#include <wallet/common/TrustIndicator.h>
#include <wallet/stellar/database/StellarLikeTransactionDatabaseHelper.hpp>

#include <algorithm>

using namespace soci;

namespace ledger {
    namespace core {

        std::vector<std::string> OperationDatabaseHelper::fetchFromBlocks(soci::session &sql, std::vector<std::string> const &blockUIDs) {
            rowset<std::string> rows = (
                sql.prepare << "SELECT uid "
                               "FROM operations AS op "
                               "WHERE block_uid IN (:uids)",
                use(blockUIDs));

            return std::vector<std::string>(rows.begin(), rows.end());
        }


        std::string OperationDatabaseHelper::createUid(const std::string &accountUid, const std::string &txId,
                                                       const api::OperationType type) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}+{}", accountUid, txId, api::to_string(type)));
        }

        void OperationDatabaseHelper::queryOperations(soci::session &sql, int32_t from, int32_t to, bool complete,
                                                      bool excludeDropped, std::vector<Operation> &out) {

        }

        std::size_t
        OperationDatabaseHelper::queryOperations(soci::session &sql,
                                                 const std::string &accountUid,
                                                 std::vector<Operation> &operations,
                                                 std::function<bool(const std::string &address)> filter) {
            rowset<row> rows = (sql.prepare <<
                                            "SELECT op.amount, op.fees, op.type, op.date, op.senders, op.recipients"
                                                    " FROM operations AS op "
                                                    " WHERE op.account_uid = :uid ORDER BY op.date",
                                                    use(accountUid));

            auto filterList = [&] (const std::vector<std::string> &list) -> bool {
                for (auto& elem : list) {
                    if (filter(elem)) {
                        return true;
                    }
                }
                return false;
            };

            std::size_t c = 0;
            for (auto& row : rows) {
                auto type = api::from_string<api::OperationType>(row.get<std::string>(2));
                auto senders = strings::split(row.get<std::string>(4), ",");
                auto recipients = strings::split(row.get<std::string>(5), ",");
                if ((type == api::OperationType::SEND && row.get_indicator(4) != i_null && filterList(senders)) ||
                    (type == api::OperationType::RECEIVE && row.get_indicator(5) != i_null && filterList(recipients))) {
                    Operation operation;

                    operation.amount = BigInt::fromHex(row.get<std::string>(0));
                    operation.fees = BigInt::fromHex(row.get<std::string>(1));
                    operation.type = type;
                    operation.date = DateUtils::fromJSON(row.get<std::string>(3));

                    operations.push_back(operation);

                    c += 1;
                }
            }
            return c;
        }

        Option<bool> OperationDatabaseHelper::isOperationInBlock(soci::session &sql, const std::string &opUid) {
            rowset<row> rows = (sql.prepare <<
                                            "SELECT block_uid "
                                            "FROM operations "
                                            "WHERE uid = :uid",
                    use(opUid));
            for (auto& row : rows) {
                return Option<bool>(!row.get<Option<std::string>>(0).getValueOr("").empty());
            }
            return Option<bool>::NONE;
        }

        void OperationDatabaseHelper::eraseDataSince(
                    soci::session &sql,
                    const std::string &accountUid,
                    const std::chrono::system_clock::time_point & date,
                    const std::string &specificOperationsTableName,
                    const std::string &specificTransactionsTableName) {
            
            rowset<std::string> rows = (sql.prepare <<
                "SELECT transaction_uid FROM " << specificOperationsTableName << " AS sop "
                "JOIN operations AS op ON sop.uid = op.uid "
                "WHERE op.account_uid = :uid AND op.date >= :date", 
                use(accountUid), use(date)
            );
            std::vector<std::string> txToDelete(rows.begin(), rows.end());
            if (!txToDelete.empty()) {
                sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date", 
                    use(accountUid), use(date);
                sql << "DELETE FROM " << specificTransactionsTableName <<
                    " WHERE transaction_uid IN (:uids)", use(txToDelete);
            }
        }

    }
}
