/*
 *
 * TezosLikeAccountDatabaseHelper
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


#include "TezosLikeAccountDatabaseHelper.h"
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <crypto/SHA256.hpp>
#include <fmt/format.h>
#include <utils/DateUtils.hpp>

using namespace soci;

namespace ledger {
    namespace core {
        void TezosLikeAccountDatabaseHelper::createAccount(soci::session &sql,
                                                           const std::string walletUid,
                                                           int32_t index,
                                                           const std::string &address) {
            auto uid = AccountDatabaseHelper::createAccountUid(walletUid, index);
            sql << "INSERT INTO tezos_accounts VALUES(:uid, :wallet_uid, :idx, :address)",use(uid), use(walletUid), use(index), use(address);
        }

        bool TezosLikeAccountDatabaseHelper::queryAccount(soci::session &sql,
                                                          const std::string &accountUid,
                                                          TezosLikeAccountDatabaseEntry &entry) {
            rowset<row> rows = (sql.prepare << "SELECT xtz.idx, xtz.address, "
                    "orig.uid, orig.address, orig.spendable, orig.delegatable, orig.public_key "
                    "FROM tezos_accounts AS xtz "
                    "LEFT JOIN tezos_originated_accounts AS orig ON xtz.uid = orig.tezos_account_uid "
                    "WHERE xtz.uid = :uid", use(accountUid));
            for (auto& row : rows) {
                if (entry.address.empty()) {
                    entry.index = row.get<int32_t>(0);
                    entry.address = row.get<std::string>(1);
                }
                // Get related originated accounts
                if (row.get_indicator(2) != i_null) {
                    TezosLikeOriginatedAccountDatabaseEntry originatedEntry;
                    originatedEntry.uid = row.get<std::string>(2);
                    originatedEntry.address = row.get<std::string>(3);
                    originatedEntry.spendable = static_cast<bool>(row.get<int>(4));
                    originatedEntry.delegatable = static_cast<bool>(row.get<int>(5));
                    if (row.get_indicator(6) != i_null) {
                        originatedEntry.publicKey = row.get<std::string>(6);
                    }
                    entry.originatedAccounts.emplace_back(originatedEntry);
                }
            }
            return !entry.address.empty();
        }

        std::string TezosLikeAccountDatabaseHelper::createOriginatedAccountUid(const std::string &xtzAccountUid, const std::string &originatedAddress) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}", xtzAccountUid, originatedAddress));
        }

        void TezosLikeAccountDatabaseHelper::updatePubKeyField(soci::session &sql, const std::string &accountUid, const std::string &pubKey) {
            sql << "UPDATE tezos_originated_accounts SET public_key = :public_key WHERE uid = :uid", use(pubKey), use(accountUid);
        }

        void TezosLikeAccountDatabaseHelper::addOriginatedAccountOperation(soci::session &sql,
                                                                           const std::string &opUid,
                                                                           const std::string &tezosTxUid,
                                                                           const std::string &originatedAccountUid) {

                sql << "INSERT INTO tezos_originated_operations VALUES(:uid, :transaction_uid, :originated_account_uid)", use(opUid), use(tezosTxUid), use(originatedAccountUid);
        }

        std::size_t
        TezosLikeAccountDatabaseHelper::queryOperations(soci::session &sql,
                                                        const std::string &accountUid,
                                                        std::vector<Operation> &operations,
                                                        std::function<bool(const std::string &address)> filter) {
            std::string query = "SELECT op.amount, op.fees, op.type, op.date, op.senders, op.recipients, op.uid "
                    "FROM operations AS op "
                    "LEFT JOIN tezos_originated_operations AS orig_op ON op.uid = orig_op.uid "
                    "WHERE op.account_uid = :uid AND orig_op.uid IS NULL ORDER BY op.date";
            rowset<row> rows = (sql.prepare << query, use(accountUid));

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
                    operations.resize(operations.size() + 1);
                    auto& operation = operations[operations.size() - 1];
                    operation.amount = BigInt::fromHex(row.get<std::string>(0));
                    operation.fees = BigInt::fromHex(row.get<std::string>(1));
                    operation.type = type;
                    operation.date = DateUtils::fromJSON(row.get<std::string>(3));
                    operation.uid = row.get<std::string>(6);
                    c += 1;
                }
            }
            return c;
        }

    }
}
