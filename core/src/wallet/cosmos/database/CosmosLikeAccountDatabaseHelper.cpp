/*
 *
 * CosmosLikeAccountDatabaseHelper
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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


#include <wallet/cosmos/database/CosmosLikeAccountDatabaseHelper.hpp>

#include <wallet/common/database/AccountDatabaseHelper.h>
#include <utils/DateUtils.hpp>
#include <database/soci-option.h>
#include <database/soci-date.h>

#include <wallet/cosmos/database/SociCosmosAmount.hpp>

using namespace soci;

namespace ledger {
    namespace core {
        void CosmosLikeAccountDatabaseHelper::createAccount(soci::session &sql,
                                                            const std::string& walletUid, int32_t index,
                                                            const std::string &address) {

            cosmos::Account acc;
            auto balances { soci::coinsToString(acc.balances) };
            std::string zero { "0" };
            auto date { DateUtils::toJSON(acc.lastUpdate) };
            auto uid = AccountDatabaseHelper::createAccountUid(walletUid, index);
            sql << "INSERT INTO cosmos_accounts VALUES(:uid, :wallet_uid, :idx, :address, :acc_type,"
                   ":acc_number, :sequence, :balances, :last_update)",
            use(uid), use(walletUid), use(index), use(address), use(acc.type), use(acc.accountNumber),
            use(acc.sequence), use(balances), use(date);
        }



        bool CosmosLikeAccountDatabaseHelper::queryAccount(soci::session &sql,
                                                           const std::string &accountUid,
                                                           CosmosLikeAccountDatabaseEntry &entry) {
            rowset<row> rows = (sql.prepare << "SELECT idx, address, account_type, account_number, "
                                               "sequence, balances, last_update "
                                               "FROM cosmos_accounts "
                                               "WHERE uid = :uid", use(
                    accountUid));
            for (auto &row : rows) {
                entry.index = row.get<int32_t>(0);
                entry.address = row.get<std::string>(1);
                auto accountType = row.get<Option<std::string>>(2);
                auto accountNumber = row.get<Option<std::string>>(3);
                auto sequence = row.get<Option<std::string>>(4);
                auto balances = row.get<Option<std::string>>(5);
                auto lastUpdate = row.get<Option<std::chrono::system_clock::time_point>>(6);

                entry.details.type = accountType.getValueOr("");
                entry.details.sequence = accountType.getValueOr("0");
                entry.details.accountNumber = accountNumber.getValueOr("0");
                if (balances.nonEmpty()) {
                    soci::stringToCoins(balances.getValue(), entry.details.balances);
                }
                entry.details.address = entry.address;
                entry.lastUpdate = lastUpdate.getValueOr({});

                return true;
            }
            return false;
        }

        void CosmosLikeAccountDatabaseHelper::updateAccount(soci::session &sql, const std::string &accountUid,
                                                            const CosmosLikeAccountDatabaseEntry &entry) {
            std::string balances = soci::coinsToString(entry.details.balances);
            sql << "UPDATE cosmos_accounts SET "
                   "account_number = :account_number,"
                   "sequence = :sequence,"
                   "balances = :balances,"
                   "account_type = :account_type,"
                   "last_update = :last_update "
                   "WHERE uid = :uid",
                   use(entry.details.accountNumber),
                   use(entry.details.sequence),
                   use(balances),
                   use(entry.details.type),
                   use(entry.lastUpdate),
                   use(accountUid);

        }

    }
}
