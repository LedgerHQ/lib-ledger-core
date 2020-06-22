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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <core/database/SociDate.hpp>
#include <core/database/SociOption.hpp>
#include <core/utils/DateUtils.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>
#include <cosmos/database/AccountDatabaseHelper.hpp>
#include <cosmos/database/SociCosmosAmount.hpp>

using namespace soci;

namespace ledger {
namespace core {
void CosmosLikeAccountDatabaseHelper::createAccount(
    soci::session &sql, const std::string &walletUid, int32_t index,
    const std::string &pubkey) {
  cosmos::Account acc;
  auto balances{soci::coinsToString(acc.balances)};
  std::string zero{"0"};
  auto date{DateUtils::toJSON(acc.lastUpdate)};
  auto uid = AccountDatabaseHelper::createAccountUid(walletUid, index);
  sql << "INSERT INTO cosmos_accounts VALUES(:uid, :wallet_uid, :idx, :pubkey, "
         ":acc_type,"
         ":acc_number, :sequence, :balances, :withdraw_address, :last_update)",
      use(uid), use(walletUid), use(index), use(pubkey), use(acc.type),
      use(acc.accountNumber), use(acc.sequence), use(balances),
      use(acc.withdrawAddress), use(date);
}

bool CosmosLikeAccountDatabaseHelper::queryAccount(
    soci::session &sql, const std::string &accountUid,
    CosmosLikeAccountDatabaseEntry &entry) {
  rowset<row> rows =
      (sql.prepare << "SELECT idx, pubkey, account_type, account_number, "
                      "sequence, balances, withdraw_address, last_update "
                      "FROM cosmos_accounts "
                      "WHERE uid = :uid",
       use(accountUid));
  const auto COL_IDX = 0;
  const auto COL_PUBKEY = 1;
  const auto COL_ACC_TYPE = 2;
  const auto COL_ACC_NUM = 3;
  const auto COL_SEQUENCE = 4;
  const auto COL_BALANCES = 5;
  const auto COL_WITHDRAW_ADDRESS = 6;
  const auto COL_LAST_UPDATE = 7;
  for (auto &row : rows) {
    entry.index = row.get<int32_t>(COL_IDX);
    entry.pubkey = row.get<std::string>(COL_PUBKEY);
    auto accountType = row.get<Option<std::string>>(COL_ACC_TYPE);
    auto accountNumber = row.get<Option<std::string>>(COL_ACC_NUM);
    auto sequence = row.get<Option<std::string>>(COL_SEQUENCE);
    auto balances = row.get<Option<std::string>>(COL_BALANCES);
    auto withdrawAddress = row.get<Option<std::string>>(COL_WITHDRAW_ADDRESS);
    auto lastUpdate =
        row.get<Option<std::chrono::system_clock::time_point>>(COL_LAST_UPDATE);

    entry.details.type = accountType.getValueOr("");
    entry.details.sequence = sequence.getValueOr("0");
    entry.details.accountNumber = accountNumber.getValueOr("0");
    if (balances.nonEmpty()) {
      soci::stringToCoins(balances.getValue(), entry.details.balances);
    }
    entry.details.withdrawAddress = withdrawAddress.getValueOr("");

    entry.details.pubkey = entry.pubkey;
    entry.lastUpdate = lastUpdate.getValueOr({});

    return true;
  }
  return false;
}

void CosmosLikeAccountDatabaseHelper::updateAccount(
    soci::session &sql, const std::string &accountUid,
    const CosmosLikeAccountDatabaseEntry &entry) {
  std::string balances = soci::coinsToString(entry.details.balances);
  sql << "UPDATE cosmos_accounts SET "
         "account_number = :account_number,"
         "sequence = :sequence,"
         "balances = :balances,"
         "account_type = :account_type,"
         "withdraw_address = :withdraw_address,"
         "last_update = :last_update "
         "WHERE uid = :uid",
      use(entry.details.accountNumber), use(entry.details.sequence),
      use(balances), use(entry.details.type),
      use(entry.details.withdrawAddress), use(entry.lastUpdate),
      use(accountUid);
}

} // namespace core
} // namespace ledger
