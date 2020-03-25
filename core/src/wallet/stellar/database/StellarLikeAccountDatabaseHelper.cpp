/*
 *
 * StellarLikeAccountDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 11/07/2019.
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

#include "StellarLikeAccountDatabaseHelper.hpp"
#include <boost-tuple.h>
#include <boost/tuple/tuple.hpp>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include "StellarLikeAssetDatabaseHelper.hpp"
#include <fmt/format.h>
#include <crypto/SHA256.hpp>
#include <database/soci-option.h>
#include <database/soci-number.h>

using namespace soci;
using namespace boost::tuples;

namespace ledger {
    namespace core {


        bool StellarLikeAccountDatabaseHelper::getAccount(soci::session &sql, const std::string &accountUid,
                                                          stellar::Account& out) {
            rowset<row> rows = (sql.prepare <<
                    "SELECT address, idx, sequence, subentries_count FROM stellar_accounts WHERE uid = :uid", use(accountUid));

            for (const auto& row : rows) {
                out.accountId = row.get<std::string>(0);
                out.accountIndex = soci::get_number<uint32_t>(row, 1);
                out.sequence = row.get<std::string>(2);
                out.subentryCount = static_cast<uint32_t>(row.get<int>(3));
                getAccountSigners(sql, accountUid, out.signers);
                return true;
            }

            return false;
        }

        void StellarLikeAccountDatabaseHelper::putAccount(soci::session &sql, const std::string &walletUid,
                                                          int32_t accountIndex, const stellar::Account &in) {
            auto accountUid = AccountDatabaseHelper::createAccountUid(walletUid, accountIndex);
            sql << "UPDATE stellar_accounts SET sequence = :sequence, subentries_count = :subentry"
                   " WHERE uid = :uid", use(in.sequence), use(in.subentryCount), use(accountUid);
            for (const auto& balance : in.balances) {
                putAccountBalance(sql, accountUid, balance);
            }
            // Clear signers
            sql << "DELETE FROM stellar_account_signers WHERE account_uid = :uid", use(accountUid);
            for (const auto& signer : in.signers) {
               putAccountSigner(sql, accountUid, signer);
            }
        }

        void StellarLikeAccountDatabaseHelper::putAccountSigner(soci::session &sql, const std::string &accountUid,
                                                                const stellar::AccountSigner &signer) {
            sql << "INSERT INTO stellar_account_signers VALUES(:auid, :w, :sk, :kt)",
            use(accountUid), use(signer.weight), use(signer.key), use(signer.type);
        }

        bool StellarLikeAccountDatabaseHelper::putAccountBalance(soci::session &sql, const std::string &accountUid,
                                                                 const stellar::Balance& balance) {
            auto assetUid = StellarLikeAssetDatabaseHelper::createAssetUid(balance.assetType, balance.assetCode, balance.assetIssuer);
            auto balanceUid = StellarLikeAccountDatabaseHelper::createAccountBalanceUid(accountUid, assetUid);
            int32_t count = -1;

            StellarLikeAssetDatabaseHelper::putAsset(sql, balance.assetType, balance.assetCode, balance.assetIssuer);

            sql << "SELECT COUNT(*) FROM stellar_account_balances WHERE uid = :uid", use(balanceUid), into(count);

            auto sellingLiabilities = balance.sellingLiabilities.map<std::string>([] (const BigInt& i) { return i.toString(); });
            auto buyingLiabilities = balance.buyingLiabilities.map<std::string>([] (const BigInt& i) { return i.toString(); });
            auto amount = balance.value.toString();
            if (count == 0) {
                sql << "INSERT INTO stellar_account_balances VALUES (:uid, :account_uid, :asset_uid, :amount, :bl, :sl)",
                       use(balanceUid), use(accountUid), use(assetUid), use(amount), use(buyingLiabilities),
                       use(sellingLiabilities);
            } else {
                sql << "UPDATE stellar_account_balances SET "
                       "amount = :amount,"
                       "buying_liabilities = :bl,"
                       "selling_liabilities = :sl "
                       "WHERE uid = :uid", use(amount), use(buyingLiabilities), use(sellingLiabilities),
                       use(balanceUid);
            }
            return count == 0;
        }

        std::string
        StellarLikeAccountDatabaseHelper::createAccountBalanceUid(const std::string &accountUid,
                                                                  const std::string &assetUid) {
            return SHA256::stringToHexHash(fmt::format("{}::{}",  accountUid, assetUid));
        }

        void StellarLikeAccountDatabaseHelper::createAccount(soci::session &sql, const std::string &walletUid,
                                                             int32_t accountIndex, const stellar::Account &in) {
            auto accountUid = AccountDatabaseHelper::createAccountUid(walletUid, accountIndex);
            sql << "INSERT INTO stellar_accounts VALUES (:uid, :wallet_uid, :idx, :address, :sequence, :count)",
                    use(accountUid), use(walletUid), use(accountIndex), use(in.accountId), use(in.sequence),
                    use(in.subentryCount);
        }

        void StellarLikeAccountDatabaseHelper::getAccountBalances(soci::session &sql, const std::string &accountUid,
                                                                  stellar::Account &out) {
            rowset<row> rows = (sql.prepare <<
                    "SELECT a.asset_type, a.asset_code, a.asset_issuer, b.amount, "
                    "b.buying_liabilities, b.selling_liabilities "
                    "FROM stellar_account_balances AS b "
                    "LEFT JOIN stellar_assets AS a ON asset_uid = a.uid "
                    "WHERE b.account_uid = :uid", use(accountUid));

            for (const auto& row : rows) {
                stellar::Balance balance;
                balance.assetType = row.get<std::string>(0);
                balance.assetCode = row.get<Option<std::string>>(1);
                balance.assetIssuer = row.get<Option<std::string>>(2);
                balance.value = BigInt::fromString(row.get<std::string>(3));
                balance.buyingLiabilities = row.get<Option<std::string>>(4).map<BigInt>([] (const std::string& s) {
                    return BigInt::fromString(s);
                });
                balance.sellingLiabilities = row.get<Option<std::string>>(4).map<BigInt>([] (const std::string& s) {
                    return BigInt::fromString(s);
                });
                out.balances.emplace_back(balance);
            }
        }

        void StellarLikeAccountDatabaseHelper::getAccountSigners(soci::session &sql, const std::string &accountUid,
                                                                 std::vector<stellar::AccountSigner> &signers) {
            rowset<row> rows = (sql.prepare <<
                    "SELECT weight, signer_key, key_type FROM stellar_account_signers WHERE account_uid = :uid",
                    use(accountUid));
            for (const auto& row : rows) {
                stellar::AccountSigner signer;
                signer.weight = get_number<int32_t>(row, 0);
                signer.key = row.get<std::string>(1);
                signer.type = row.get<std::string>(2);
                signers.emplace_back(std::move(signer));
            }
        }

    }
}