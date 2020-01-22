/*
 *
 * migrations
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/04/2017.
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

#include <core/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr CoreMigration::COIN_ID;
        uint32_t constexpr CoreMigration::CURRENT_VERSION;

        void setupMigrations(soci::session& sql) {
            sql << "CREATE TABLE __database_meta__("
                "id INT PRIMARY KEY NOT NULL,"
                "version INT NOT NULL"
            ")";
        }

        void unsetupMigrations(soci::session& sql) {
            sql << "DROP TABLE __database_meta__";
        }

        template <> void migrate<1, CoreMigration>(soci::session& sql) {
            // Abstract currency table
            sql << "CREATE TABLE currencies("
                "name VARCHAR(255) PRIMARY KEY NOT NULL,"
                "bip44_coin_type INTEGER NOT NULL,"
                "payment_uri_scheme VARCHAR(255)"
            ")";

            // Abstract units table
            sql << "CREATE TABLE units("
                "name VARCHAR(255) NOT NULL,"
                "magnitude INTEGER NOT NULL,"
                "code VARCHAR(255) NOT NULL,"
                "currency_name VARCHAR(255) REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE"
            ")";

            // Abstract wallet table
            sql << "CREATE TABLE wallets("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "name VARCHAR(255),"
                "tenant VARCHAR(255),"
                "currency_name VARCHAR(255) NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                "configuration TEXT NOT NULL, "
                "created_at TEXT NOT NULL"
            ")";

            // Abstract account table
            sql << "CREATE TABLE accounts("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "idx INTEGER NOT NULL,"
                "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE, "
                "created_at TEXT NOT NULL"
            ")";

            // Abstract block table
            sql << "CREATE TABLE blocks("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "hash VARCHAR(255) NOT NULL,"
                    "height BIGINT NOT NULL,"
                    "time VARCHAR(255) NOT NULL,"
                    "currency_name VARCHAR(255)"
            ")";

            // Abstract operation table
            sql << "CREATE TABLE operations("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "account_uid VARCHAR(255) NOT NULL REFERENCES accounts(uid) ON DELETE CASCADE,"
                "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE,"
                "type VARCHAR(255) NOT NULL,"
                "date VARCHAR(255) NOT NULL,"
                "senders TEXT NOT NULL,"
                "recipients TEXT NOT NULL,"
                "amount VARCHAR(255) NOT NULL,"
                "fees VARCHAR(255),"
                "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                "currency_name VARCHAR(255) NOT NULL REFERENCES currencies(name) ON DELETE CASCADE,"
                "trust TEXT"
            ")";
        }

        template <> void rollback<1, CoreMigration>(soci::session& sql) {
            // Abstract operation table
            sql << "DROP TABLE operations";

            // Abstract block table
            sql << "DROP TABLE blocks";

            // Abstract account table
            sql << "DROP TABLE accounts";

            // Abstract wallet table
            sql << "DROP TABLE wallets";

            // Abstract units table
            sql << "DROP TABLE units";

            // Abstract currency table
            sql << "DROP TABLE currencies";
        }
    }
}
