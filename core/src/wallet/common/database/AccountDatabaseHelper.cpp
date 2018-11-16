/*
 *
 * AccountDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/05/2017.
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
#include "AccountDatabaseHelper.h"
#include <crypto/SHA256.hpp>
#include <fmt/format.h>
#include <list>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <utils/DateUtils.hpp>

using namespace soci;

namespace ledger {
    namespace core {

        std::string AccountDatabaseHelper::createAccountUid(const std::string &walletUid, int32_t accountIndex) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}", walletUid, accountIndex));
        }

        void AccountDatabaseHelper::createAccount(soci::session &sql, const std::string &walletUid, int32_t index) {
            auto uid = createAccountUid(walletUid, index);
            sql << "INSERT INTO accounts VALUES(:uid, :idx, :wallet_uid, :now)", use(uid), use(index), use(walletUid),
                    use(DateUtils::now());
        }

        void AccountDatabaseHelper::removeAccount(soci::session &sql, const std::string &walletUid, int32_t index) {
            auto uid = createAccountUid(walletUid, index);
            sql << "DELETE FROM accounts WHERE uid = :uid", use(uid);
        }

        bool AccountDatabaseHelper::accountExists(soci::session &sql, const std::string &walletUid, int32_t index) {
            auto uid = createAccountUid(walletUid, index);
            int64_t count = 0L;
            sql << "SELECT COUNT(*) FROM accounts WHERE uid = :uid", use(uid), into(count);
            return count == 1L;
        }

        int32_t AccountDatabaseHelper::getAccountsCount(soci::session &sql, const std::string &walletUid) {
            int32_t count = 0;
            sql << "SELECT COUNT(*) FROM accounts WHERE wallet_uid = :walletUid", use(walletUid), into(count);
            return count;
        }



        int32_t AccountDatabaseHelper::computeNextAccountIndex(soci::session &sql, const std::string &walletUid) {
            //TODO: Enhance performance for huge wallets by reducing the select range.
            int32_t currentIndex = 0;
            rowset<int32_t> rows = (sql.prepare << "SELECT idx FROM accounts WHERE wallet_uid = :uid ORDER BY idx", use(walletUid));
            for (auto& idx : rows) {
                if (idx > currentIndex) {
                    return currentIndex;
                } else if (idx == currentIndex) {
                    currentIndex += 1;
                }
            }
            return currentIndex;
        }

        std::list<int32_t>&
        AccountDatabaseHelper::getAccountsIndexes(soci::session &sql, const std::string &walletUid, int32_t from,
                                                  int32_t count, std::list<int32_t>& out) {
            rowset<int32_t> rows = (sql.prepare <<
                    "SELECT idx FROM accounts WHERE wallet_uid = :uid ORDER BY created_at LIMIT :count OFFSET :off"
                    "",
                    use(walletUid), use(count), use(from));
            for (auto& idx : rows) {
                out.push_back(idx);
            }
            return out;
        }

        Option<api::Block> AccountDatabaseHelper::getLastBlockWithOperations(soci::session &sql, const std::string &accountUid) {
            //Get block_uid of most recent operation from DB
            rowset<row> rows = (sql.prepare << "SELECT op.block_uid, b.hash, b.height, b.time, b.currency_name "
                                                    "FROM operations AS op "
                                                    "JOIN blocks AS b ON op.block_uid = b.uid "
                                                    "WHERE op.account_uid = :uid ORDER BY op.date DESC LIMIT 1",
                                                    use(accountUid));
            for (auto& row : rows) {
                auto block_uid = row.get<std::string>(0);
                auto hash = row.get<std::string>(1);
                auto height = get_number<int64_t>(row, 2);
                auto time = row.get<std::chrono::system_clock::time_point>(3);
                return Option<api::Block>(
                        api::Block(hash, block_uid, time, row.get<std::string>(4), height)
                );
            }
            return Option<api::Block>();
        }


    }
}