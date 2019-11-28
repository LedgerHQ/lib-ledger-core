/*
 *
 * WalletDatabaseHelper
 * ledger-core
 *
 * Created by Dimitri Sabadie on 2019/11/05.
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

#include <core/database/SociDate.hpp>
#include <core/wallet/WalletDatabaseHelper.hpp>
#include <core/utils/DateUtils.hpp>

using namespace soci;

namespace ledger {
    namespace core {
        void WalletDatabaseHelper::putWallet(
            soci::session &sql,
            const WalletDatabaseEntry &wallet
        ) {
            auto serializeConfig = wallet.configuration->serialize();
            auto configuration = hex::toString(serializeConfig);
            auto now = DateUtils::now();
            if (!walletExists(sql, wallet)) {
                sql << "INSERT INTO wallets VALUES(:uid, :name, :tenant, :currency_name, :configuration, :now)",
                use(wallet.uid),
                use(wallet.name),
                use(wallet.tenant),
                use(wallet.currencyName),
                use(configuration),
                use(now);
            } else {
                sql << "UPDATE wallets SET configuration = :configuration WHERE uid = :uid",
                use(configuration), use(wallet.uid);
            }
        }

        int64_t WalletDatabaseHelper::getWallets(
            soci::session &sql,
            int64_t offset,
            std::vector<WalletDatabaseEntry> &wallets
        ) {
            rowset<row> rows = (
                sql.prepare <<  "SELECT uid, name, currency_name, configuration, tenant "
                                "FROM wallets "
                                "ORDER BY created_at "
                                "LIMIT :count OFFSET :offset",
                use(wallets.size()),
                use(offset)
            );

            int64_t index = 0;
            for (auto& row : rows) {
                WalletDatabaseEntry entry;
                inflateWalletEntry(row, entry);
                wallets[index] = entry;
                index += 1;
            }

            return index;
        }

        int64_t WalletDatabaseHelper::getWalletCount(
            soci::session& sql
        ) {
            int64_t count;
            sql << "SELECT COUNT(*) FROM wallets", into(count);
            return count;
        }

        bool WalletDatabaseHelper::getWallet(
            soci::session &sql,
            const std::string& tenant,
            const std::string& walletName,
            WalletDatabaseEntry &entry
        ) {
            auto walletUid = WalletDatabaseEntry::createWalletUid(tenant, walletName);

            rowset<row> rows = (
                sql.prepare << "SELECT uid, name, currency_name, configuration, tenant "
                               "FROM wallets "
                               "WHERE uid = :uid",
                use(walletUid)
            );

            for (auto& row : rows) {
                inflateWalletEntry(row, entry);
                return true;
            }

            return false;
        }

        void WalletDatabaseHelper::inflateWalletEntry(
            soci::row &row,
            WalletDatabaseEntry &entry
        ) {
            entry.uid = row.get<std::string>(0);
            entry.name = row.get<std::string>(1);
            entry.currencyName = row.get<std::string>(2);

            auto serializedConfig = hex::toByteArray(row.get<std::string>(3));

            entry.configuration = std::static_pointer_cast<ledger::core::DynamicObject>(DynamicObject::load(serializedConfig));
            entry.tenant = row.get<std::string>(4);
        }

        bool WalletDatabaseHelper::removeWallet(
            soci::session &sql,
            const WalletDatabaseEntry &entry
        ) {
            if (!walletExists(sql, entry)) {
                return false;
            }

            sql << "DELETE FROM wallets WHERE uid = :uid", use(entry.uid);
            return true;
        }

        bool WalletDatabaseHelper::walletExists(
            soci::session &sql,
            const WalletDatabaseEntry &entry
        ) {
            int count = 0;
            sql << "SELECT COUNT(*) FROM wallets WHERE uid = :uid", use(entry.uid), into(count);
            return count > 0;
        }
    }
}
