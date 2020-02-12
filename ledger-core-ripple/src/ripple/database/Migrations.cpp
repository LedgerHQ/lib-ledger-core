/*
 *
 * migrations.cpp
 *
 * Created by Dimitri Sabadie on 2019/07/04.
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

#include <ripple/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr XRPMigration::COIN_ID;
        uint32_t constexpr XRPMigration::CURRENT_VERSION;

        template <> void migrate<1, XRPMigration>(soci::session& sql) {
            sql << "CREATE TABLE ripple_currencies("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "identifier VARCHAR(255) NOT NULL,"
                    "xpub_version VARCHAR(255) NOT NULL,"
                    "message_prefix VARCHAR(255) NOT NULL,"
                    "additional_RIPs TEXT"
                    ")";

            sql << "CREATE TABLE ripple_accounts("
                    "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "idx INTEGER NOT NULL,"
                    "address VARCHAR(255) NOT NULL"
                    ")";

            sql << "CREATE TABLE ripple_transactions("
                    "transaction_uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "hash VARCHAR(255) NOT NULL,"
                    "value VARCHAR(255) NOT NULL,"
                    "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                    "time VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "receiver VARCHAR(255) NOT NULL,"
                    "fees VARCHAR(255) NOT NULL,"
                    "confirmations BIGINT NOT NULL"
                    ")";

            sql << "CREATE TABLE ripple_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "transaction_uid VARCHAR(255) NOT NULL REFERENCES ripple_transactions(transaction_uid),"
                    "transaction_hash VARCHAR(255) NOT NULL"
                    ")";
        }

        template <> void rollback<1, XRPMigration>(soci::session& sql) {
            sql << "DROP TABLE ripple_operations";
            sql << "DROP TABLE ripple_transactions";
            sql << "DROP TABLE ripple_accounts";
            sql << "DROP TABLE ripple_currencies";
        }

        template <> void migrate<2, XRPMigration>(soci::session& sql) {
            sql << "CREATE TABLE ripple_memos("
                   "transaction_uid VARCHAR(255) NOT NULL REFERENCES ripple_transactions(transaction_uid) ON DELETE CASCADE,"
                   "data VARCHAR(1024),"
                   "fmt VARCHAR(1024),"
                   "ty VARCHAR(1024),"
                   "array_index INTEGER NOT NULL"
                   ")";
        }

        template <> void rollback<2, XRPMigration>(soci::session& sql) {
            sql << "DROP TABLE ripple_memos";
        }

        template <> void migrate<3, XRPMigration>(soci::session& sql) {
            sql << "ALTER TABLE ripple_transactions ADD COLUMN sequence BIGINT";
        }

        template <> void rollback<3, XRPMigration>(soci::session& sql) {
        }

        template <> void migrate<4, XRPMigration>(soci::session& sql) {
            sql << "ALTER TABLE ripple_transactions ADD COLUMN destination_tag BIGINT";
        }

        template <> void rollback<4, XRPMigration>(soci::session& sql) {
        }

        template <> void migrate<5, XRPMigration>(soci::session& sql) {
            // 1 if success, 0 otherwise
            // <https://xrpl.org/transaction-results.html>
            sql << "ALTER TABLE ripple_transactions ADD COLUMN status INTEGER";
        }

        template <> void rollback<5, XRPMigration>(soci::session& sql) {
        }
    }
}
