/*
 * AlgorandAccountDatabaseHelper
 *
 * Created by Hakim Aammar on 18/05/2020.
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

#include "AlgorandAccountDatabaseHelper.hpp"
#include "../model/AlgorandAccount.hpp"
#include "../operations/AlgorandOperation.hpp"

#include <wallet/common/AbstractAccount.hpp>
#include <database/soci-number.h>
#include <collections/vector.hpp>
#include <utils/Assert.hpp>

namespace ledger {
namespace core {
namespace algorand {

    std::string AccountDatabaseHelper::createAccount(soci::session & sql,
                                                     const std::string & walletUid,
                                                     const AccountDatabaseEntry & account) {

        ledger::core::AccountDatabaseHelper::createAccount(sql, walletUid, account.index);

        auto uid = createAccountUid(walletUid, account.index);

        sql << "INSERT INTO algorand_accounts VALUES(:uid, :wallet_uid, :idx, :address)",
            soci::use(uid),
            soci::use(walletUid),
            soci::use(account.index),
            soci::use(account.address);

        return uid;
    }

    bool AccountDatabaseHelper::queryAccount(soci::session & sql,
                                             const std::string & accountUid,
                                             AccountDatabaseEntry & account) {

        soci::rowset<soci::row> rows = (sql.prepare << "SELECT idx, address "
                                                       "FROM algorand_accounts "
                                                       "WHERE uid = :uid", soci::use(accountUid));
        auto results = vector::fromRowset<AccountDatabaseEntry>(rows,
            [](soci::row &row) -> AccountDatabaseEntry {
                AccountDatabaseEntry accountRow;
                accountRow.index = soci::get_number<int32_t>(row, 0);
                accountRow.address = row.get<std::string>(1);
                return accountRow;
            }
        );

        // Assert we retreived only one account, and retrieve info from that single account
        assertSingleRow<AccountDatabaseEntry>(results, fmt::format("More than one account found with uid '{}'.", accountUid));
        account.index = results.begin()->index;
        account.address = results.begin()->address;

        if (results.empty()) {
            return false;
        } else if (results.size() == 1) {
           return !account.address.empty();
        } else {
            throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "More than one account found with uid '{}'.", accountUid);
        }
    }

}
}
}
