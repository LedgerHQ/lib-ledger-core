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
#include "migrations.hpp"

namespace ledger {
    namespace core {

        template <> bool migrate<-1>(soci::session& sql, int currentVersion) {
            return false;
        }

        template <> void migrate<0>(soci::session& sql) {
            sql << "CREATE TABLE __database_meta__("
                "id INT PRIMARY KEY NOT NULL,"
                "version INT NOT NULL"
            ")";
            sql << "INSERT INTO __database_meta__(id, version) VALUES(0, 0)";
        }

        template <> void migrate<1>(soci::session& sql) {

            // Pool table
            sql << "CREATE TABLE pools("
                "name VARCHAR(255) PRIMARY KEY NOT NULL,"
                "created_at VARCHAR(255) NOT NULL"
            ")";

            // Abstract currency table
            sql << "CREATE TABLE currencies("
                "name VARCHAR(255) PRIMARY KEY NOT NULL,"
                "type VARCHAR(255) NOT NULL,"
                "bip44_coin_type INTEGER NOT NULL,"
                "payment_uri_scheme VARCHAR(255)"
            ")";

            // Abstract units table
            sql << "CREATE TABLE units("
                "name VARCHAR(255) NOT NULL,"
                "magnitude INTEGER NOT NULL,"
                "symbol VARCHAR(255) NOT NULL,"
                "code VARCHAR(255) NOT NULL,"
                "currency_name VARCHAR(255) REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE"
            ")";

            // Abstract wallet table
            sql << "CREATE TABLE wallets("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "name VARCHAR(255),"
                "currency_name VARCHAR(255) NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                "pool_name VARCHAR(255) NOT NULL REFERENCES pools(name) ON DELETE CASCADE ON UPDATE CASCADE,"
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
                "amount BIGINT NOT NULL,"
                "fees BIGINT,"
                "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                "currency_name VARCHAR(255) NOT NULL REFERENCES currencies(name) ON DELETE CASCADE,"
                "trust TEXT"
            ")";

            // Bitcoin currency table

            sql << "CREATE TABLE bitcoin_currencies("
                "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                "identifier VARCHAR(255) NOT NULL,"
                "p2pkh_version VARCHAR(255) NOT NULL,"
                "p2sh_version VARCHAR(255) NOT NULL,"
                "xpub_version VARACHAR(255) NOT NULL,"
                "dust_amount BIGINT NOT NULL,"
                "fee_policy VARCHAR(20) NOT NULL,"
                "message_prefix VARCHAR(255) NOT NULL,"
                "has_timestamped_transaction INTEGER NOT NULL"
            ")";

            // Bitcoin transaction table
            sql << "CREATE TABLE bitcoin_transactions("
                "transaction_uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "hash VARCHAR(255) NOT NULL,"
                "version INTEGER,"
                "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                "time VARCHAR(255),"
                "locktime INTEGER"
            ")";

            // Bitcoin input table
            sql << "CREATE TABLE bitcoin_inputs("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL," // accountId_idx_previoustxhash_coinbase
                "previous_output_idx INTEGER,"
                "previous_tx_hash VARCHAR(255),"
                "previous_tx_uid VARCHAR(255),"
                "amount  BIGINT,"
                "address VARCHAR(255),"
                "coinbase VARCHAR(255),"
                "sequence BIGINT NOT NULL"
            ")";

            // Bitcoin output table
            sql << "CREATE TABLE bitcoin_outputs("
                "idx INTEGER NOT NULL,"
                "transaction_uid VARCHAR(255) NOT NULL REFERENCES bitcoin_transactions(transaction_uid) ON DELETE CASCADE,"
                "transaction_hash VARCHAR(255) NOT NULL,"
                "amount BIGINT NOT NULL,"
                "script TEXT NOT NULL,"
                "address VARCHAR(255),"
                "account_uid VARCHAR(255),"
                "PRIMARY KEY (idx, transaction_uid)"
                ")";

            // Bitcoin transaction <-> input table
            sql << "CREATE TABLE bitcoin_transaction_inputs("
                "transaction_uid VARCHAR(255) NOT NULL REFERENCES bitcoin_transactions(transaction_uid) ON DELETE CASCADE,"
                "transaction_hash VARCHAR(255) NOT NULL,"
                "input_uid VARCHAR(255) NOT NULL REFERENCES bitcoin_inputs(uid) ON DELETE CASCADE,"
                "input_idx INTEGER NOT NULL,"
                "PRIMARY KEY (transaction_uid, input_uid)"
                ")";

            // Bitcoin account
            sql << "CREATE TABLE bitcoin_accounts("
                "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "idx INTEGER NOT NULL,"
                "xpub VARCHAR(255) NOT NULL"
            ")";

            // Bitcoin operation table

            sql << "CREATE TABLE bitcoin_operations("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                "transaction_uid VARCHAR(255) NOT NULL REFERENCES bitcoin_transactions(transaction_uid),"
                "transaction_hash VARCHAR(255) NOT NULL"
                ")";
        }

        template <> void migrate<2>(soci::session& sql) {
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN timestamp_delay BIGINT DEFAULT 0";
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN sighash_type VARCHAR(255) DEFAULT 01";
        }

        template <> void migrate<3>(soci::session& sql) {
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN additional_BIPs TEXT DEFAULT ''";
        }

        template <> void migrate<4>(soci::session& sql) {
            auto count = 0;
            sql << "SELECT COUNT(*) FROM bitcoin_currencies WHERE identifier = 'dgb'", soci::into(count);
            if (count > 0) {
                sql << "UPDATE bitcoin_currencies SET p2sh_version = '3f' WHERE identifier = 'dgb' ";
            }
        }

        template <> void migrate<5>(soci::session& sql) {

            sql << "CREATE TABLE ethereum_currencies("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "identifier VARCHAR(255) NOT NULL,"
                    "xpub_version VARACHAR(255) NOT NULL,"
                    "message_prefix VARCHAR(255) NOT NULL,"
                    "additional_EIPs TEXT"
                    ")";

            sql << "CREATE TABLE ethereum_accounts("
                    "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "idx INTEGER NOT NULL,"
                    "address VARCHAR(255) NOT NULL"
                    ")";

            sql << "CREATE TABLE ethereum_transactions("
                    "transaction_uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "hash VARCHAR(255) NOT NULL,"
                    "nonce VARCHAR(255) NOT NULL,"
                    "value BIGINT NOT NULL,"
                    "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                    "time VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "receiver VARCHAR(255) NOT NULL,"
                    "input_data VARCHAR(255),"
                    "gas_price BIGINT NOT NULL,"
                    "gas_limit BIGINT NOT NULL,"
                    "gas_used BIGINT NOT NULL,"
                    "confirmations BIGINT NOT NULL,"
                    "status BIGINT NOT NULL"
                    ")";

            sql << "CREATE TABLE ethereum_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "transaction_uid VARCHAR(255) NOT NULL REFERENCES ethereum_transactions(transaction_uid),"
                    "transaction_hash VARCHAR(255) NOT NULL"
                    ")";

            sql << "CREATE TABLE erc20_accounts("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL ,"
                    "ethereum_account_uid VARCHAR(255) NOT NULL REFERENCES ethereum_accounts(uid) ON DELETE CASCADE,"
                    "contract_address VARCHAR(255) NOT NULL"
                    ")";

            sql << "CREATE TABLE erc20_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL ,"
                    "ethereum_operation_uid VARCHAR(255) NOT NULL REFERENCES ethereum_operations(uid) ON DELETE CASCADE,"
                    "erc20_account_uid VARCHAR(255) NOT NULL REFERENCES erc20_accounts(uid) ON DELETE CASCADE,"
                    "hash VARCHAR(255) NOT NULL,"
                    "nonce VARCHAR(255) NOT NULL,"
                    "value BIGINT NOT NULL,"
                    "time VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "receiver VARCHAR(255) NOT NULL,"
                    "input_data VARCHAR(255),"
                    "gas_price BIGINT NOT NULL,"
                    "gas_limit BIGINT NOT NULL,"
                    "gas_used BIGINT NOT NULL,"
                    "status BIGINT NOT NULL"
                    ")";

            sql << "CREATE TABLE erc20_tokens("
                    "contract_address VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "name VARCHAR(255) NOT NULL,"
                    "symbol VARCHAR(255) NOT NULL,"
                    "number_of_decimal INTEGER NOT NULL"
                    ")";

        }

    }
}