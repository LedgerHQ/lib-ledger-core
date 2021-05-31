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
        int getDatabaseMigrationVersion(soci::session& sql) {
            int version = -1;

            try {
                soci::statement st = (sql.prepare << "SELECT version FROM __database_meta__ WHERE id = 0", soci::into(version));
                st.execute();
                st.fetch();
            } catch (...) {
                // if we cannot find the version, it stays set to -1
            }

            return version;
        }

        template <> bool migrate<-1>(soci::session& sql, int currentVersion, api::DatabaseBackendType type) {
            return false;
        }

        template <> void rollback<-1>(soci::session& sql, int currentVersion, api::DatabaseBackendType type) {
        }

        template <> void migrate<0>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "CREATE TABLE __database_meta__("
                "id INT PRIMARY KEY NOT NULL,"
                "version INT NOT NULL"
            ")";

            sql << "INSERT INTO __database_meta__(id, version) VALUES(0, 0)";
        }

        template <> void rollback<0>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE __database_meta__";
        }

        template <> void migrate<1>(soci::session& sql, api::DatabaseBackendType type) {
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
                "amount VARCHAR(255) NOT NULL,"
                "fees VARCHAR(255),"
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
                "xpub_version VARCHAR(255) NOT NULL,"
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

        template <> void rollback<1>(soci::session& sql, api::DatabaseBackendType type) {
            // Bitcoin operation table
            sql << "DROP TABLE bitcoin_operations";

            // Bitcoin account
            sql << "DROP TABLE bitcoin_accounts";

            // Bitcoin transaction <-> input table
            sql << "DROP TABLE bitcoin_transaction_inputs";

            // Bitcoin output table
            sql << "DROP TABLE bitcoin_outputs";

            // Bitcoin input table
            sql << "DROP TABLE bitcoin_inputs";

            // Bitcoin transaction table
            sql << "DROP TABLE bitcoin_transactions";

            // Bitcoin currency table
            sql << "DROP TABLE bitcoin_currencies";

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

            // Pool table
            sql << "DROP TABLE pools";
        }

        template <> void migrate<2>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN timestamp_delay BIGINT DEFAULT 0";
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN sighash_type VARCHAR(255) DEFAULT 01";
        }

        template <> void rollback<2>(soci::session& sql, api::DatabaseBackendType type) {
            // not supported in standard ways by SQLite :(
        }

        template <> void migrate<3>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN additional_BIPs TEXT DEFAULT ''";
        }

        template <> void rollback<3>(soci::session& sql, api::DatabaseBackendType type) {
            // not supported in standard ways by SQLite :(
        }

        template <> void migrate<4>(soci::session& sql, api::DatabaseBackendType type) {
            auto count = 0;
            sql << "SELECT COUNT(*) FROM bitcoin_currencies WHERE identifier = 'dgb'", soci::into(count);
            if (count > 0) {
                sql << "UPDATE bitcoin_currencies SET p2sh_version = '3f' WHERE identifier = 'dgb' ";
            }
        }

        template <> void rollback<4>(soci::session& sql, api::DatabaseBackendType type) {
            // cannot rollback
        }

        template <> void migrate<5>(soci::session& sql, api::DatabaseBackendType type) {
            // ETH currencies
            sql << "CREATE TABLE ethereum_currencies("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "identifier VARCHAR(255) NOT NULL,"
                    "chain_id VARCHAR(255) NOT NULL,"
                    "xpub_version VARCHAR(255) NOT NULL,"
                    "message_prefix VARCHAR(255) NOT NULL,"
                    "additional_EIPs TEXT"
                    ")";

            // ETH accounts
            sql << "CREATE TABLE ethereum_accounts("
                    "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "idx INTEGER NOT NULL,"
                    "address VARCHAR(255) NOT NULL"
                    ")";

            // ETH transactions
            sql << "CREATE TABLE ethereum_transactions("
                    "transaction_uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "hash VARCHAR(255) NOT NULL,"
                    "nonce VARCHAR(255) NOT NULL,"
                    "value VARCHAR(255) NOT NULL,"
                    "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                    "time VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "receiver VARCHAR(255) NOT NULL,"
                    "input_data VARCHAR(255),"
                    "gas_price VARCHAR(255) NOT NULL,"
                    "gas_limit VARCHAR(255) NOT NULL,"
                    "gas_used VARCHAR(255) NOT NULL,"
                    "confirmations BIGINT NOT NULL,"
                    "status BIGINT NOT NULL"
                    ")";

            // ETH operations
            sql << "CREATE TABLE ethereum_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "transaction_uid VARCHAR(255) NOT NULL REFERENCES ethereum_transactions(transaction_uid) ON DELETE CASCADE,"
                    "transaction_hash VARCHAR(255) NOT NULL"
                    ")";

            // ERC20 accounts
            sql << "CREATE TABLE erc20_accounts("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL ,"
                    "ethereum_account_uid VARCHAR(255) NOT NULL REFERENCES ethereum_accounts(uid) ON DELETE CASCADE,"
                    "contract_address VARCHAR(255) NOT NULL"
                    ")";

            // ERC20 operations
            sql << "CREATE TABLE erc20_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL ,"
                    "ethereum_operation_uid VARCHAR(255) NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "account_uid VARCHAR(255) NOT NULL REFERENCES erc20_accounts(uid) ON DELETE CASCADE,"
                    "type VARCHAR(255) NOT NULL,"
                    "hash VARCHAR(255) NOT NULL,"
                    "nonce VARCHAR(255) NOT NULL,"
                    "value VARCHAR(255) NOT NULL,"
                    "date VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "receiver VARCHAR(255) NOT NULL,"
                    "input_data VARCHAR(255),"
                    "gas_price VARCHAR(255) NOT NULL,"
                    "gas_limit VARCHAR(255) NOT NULL,"
                    "gas_used VARCHAR(255) NOT NULL,"
                    "status INTEGER NOT NULL"
                    ")";

            // ERC20 tokens
            sql << "CREATE TABLE erc20_tokens("
                    "contract_address VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "name VARCHAR(255) NOT NULL,"
                    "symbol VARCHAR(255) NOT NULL,"
                    "number_of_decimal INTEGER NOT NULL"
                    ")";

        }

        template <> void rollback<5>(soci::session& sql, api::DatabaseBackendType type) {
            // ERC20 tokens
            sql << "DROP TABLE erc20_tokens";

            // ERC20 operations
            sql << "DROP TABLE erc20_operations";

            // ERC20 accounts
            sql << "DROP TABLE erc20_accounts";

            // ETH transactions and operations
            switch (type) {
                case api::DatabaseBackendType::POSTGRESQL: {
                    sql << "DROP TABLE ethereum_operations CASCADE";
                    sql << "DROP TABLE ethereum_transactions CASCADE";
                    break;
                }
                default: {
                    sql << "DROP TABLE ethereum_operations";
                    sql << "DROP TABLE ethereum_transactions";
                    break;
                }
            }
            // ETH accounts
            sql << "DROP TABLE ethereum_accounts";

            // ETH currencies
            sql << "DROP TABLE ethereum_currencies";
        }

        template <> void migrate<6>(soci::session& sql, api::DatabaseBackendType type) {
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

        template <> void rollback<6>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE ripple_operations";

            sql << "DROP TABLE ripple_transactions";

            sql << "DROP TABLE ripple_accounts";

            sql << "DROP TABLE ripple_currencies";
        }

        template <> void migrate<7>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "CREATE TABLE bech32_parameters("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES bitcoin_currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "hrp VARCHAR(255) NOT NULL,"
                    "separator VARCHAR(255) NOT NULL,"
                    "generator VARCHAR(255) NOT NULL,"
                    "p2wpkh_version VARCHAR(255) NOT NULL,"
                    "p2wsh_version VARCHAR(255) NOT NULL"
                    ")";
        }

        template <> void rollback<7>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE bech32_parameters";
        }

        template <> void migrate<8>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "CREATE TABLE ripple_memos("
                   "transaction_uid VARCHAR(255) NOT NULL REFERENCES ripple_transactions(transaction_uid) ON DELETE CASCADE,"
                   "data VARCHAR(1024),"
                   "fmt VARCHAR(1024),"
                   "ty VARCHAR(1024),"
                   "array_index INTEGER NOT NULL"
                   ")";
        }

        template <> void rollback<8>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE ripple_memos";
        }

        template <> void migrate<9>(soci::session& sql, api::DatabaseBackendType type) {
            // Since ALTER TABLE for changing data is non standard we have no choice but create a swap table, migrate all data from the legacy table to the swap and
            // then remove the legacy table and rename the swap table to the final table name. We are doing this to change input_data for ET and ERC txs data type from
            // VARCHAR(255) to TEXT since nothings prevents those fields to be bigger than 255 characters long.

             // ETH transactions
            sql << "CREATE TABLE eth_swap("
                "transaction_uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "hash VARCHAR(255) NOT NULL,"
                "nonce VARCHAR(255) NOT NULL,"
                "value VARCHAR(255) NOT NULL,"
                "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                "time VARCHAR(255) NOT NULL,"
                "sender VARCHAR(255) NOT NULL,"
                "receiver VARCHAR(255) NOT NULL,"
                "input_data TEXT,"
                "gas_price VARCHAR(255) NOT NULL,"
                "gas_limit VARCHAR(255) NOT NULL,"
                "gas_used VARCHAR(255) NOT NULL,"
                "confirmations BIGINT NOT NULL,"
                "status BIGINT NOT NULL"
                ")";

             sql << "INSERT INTO eth_swap "
                "SELECT transaction_uid, hash, nonce, value, block_uid, time, sender, receiver, input_data, gas_price, gas_limit, gas_used, confirmations, status "
                "FROM ethereum_transactions";

            switch (type) {
                case api::DatabaseBackendType::POSTGRESQL: {
                    sql << "DROP TABLE ethereum_transactions CASCADE";
                    break;
                }
                default: {
                    sql << "DROP TABLE ethereum_transactions";
                    break;
                }
            }

            sql << "ALTER TABLE eth_swap RENAME TO ethereum_transactions";

             // ERC20 operations
            sql << "CREATE TABLE erc20_swap( "
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "ethereum_operation_uid VARCHAR(255) NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                "account_uid VARCHAR(255) NOT NULL REFERENCES erc20_accounts(uid) ON DELETE CASCADE,"
                "type VARCHAR(255) NOT NULL,"
                "hash VARCHAR(255) NOT NULL,"
                "nonce VARCHAR(255) NOT NULL,"
                "value VARCHAR(255) NOT NULL,"
                "date VARCHAR(255) NOT NULL,"
                "sender VARCHAR(255) NOT NULL,"
                "receiver VARCHAR(255) NOT NULL,"
                "input_data TEXT,"
                "gas_price VARCHAR(255) NOT NULL,"
                "gas_limit VARCHAR(255) NOT NULL,"
                "gas_used VARCHAR(255) NOT NULL,"
                "status INTEGER NOT NULL"
                ")";

            sql << "INSERT INTO erc20_swap "
                "SELECT uid, ethereum_operation_uid, account_uid, type, hash, nonce, value, date, sender, receiver, input_data, gas_price, gas_limit, gas_used, status "
                "FROM erc20_operations";
            sql << "DROP TABLE erc20_operations";
            sql << "ALTER TABLE erc20_swap RENAME TO erc20_operations";
        }

        template <> void rollback<9>(soci::session& sql, api::DatabaseBackendType type) {
             // ETH transactions
            sql << "CREATE TABLE eth_swap("
                "transaction_uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "hash VARCHAR(255) NOT NULL,"
                "nonce VARCHAR(255) NOT NULL,"
                "value VARCHAR(255) NOT NULL,"
                "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                "time VARCHAR(255) NOT NULL,"
                "sender VARCHAR(255) NOT NULL,"
                "receiver VARCHAR(255) NOT NULL,"
                "input_data VARCHAR(255),"
                "gas_price VARCHAR(255) NOT NULL,"
                "gas_limit VARCHAR(255) NOT NULL,"
                "gas_used VARCHAR(255) NOT NULL,"
                "confirmations BIGINT NOT NULL,"
                "status BIGINT NOT NULL"
                ")";

             sql << "INSERT INTO eth_swap "
                "SELECT transaction_uid, hash, nonce, value, block_uid, time, sender, receiver, input_data, gas_price, gas_limit, gas_used, confirmations, status "
                "FROM ethereum_transactions";

            switch (type) {
                case api::DatabaseBackendType::POSTGRESQL: {
                    sql << "DROP TABLE ethereum_transactions CASCADE";
                    break;
                }
                default: {
                    sql << "DROP TABLE ethereum_transactions";
                    break;
                }
            }

            sql << "ALTER TABLE eth_swap RENAME TO ethereum_transactions";

             // ERC20 operations
            sql << "CREATE TABLE erc20_swap( "
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "ethereum_operation_uid VARCHAR(255) NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                "account_uid VARCHAR(255) NOT NULL REFERENCES erc20_accounts(uid) ON DELETE CASCADE,"
                "type VARCHAR(255) NOT NULL,"
                "hash VARCHAR(255) NOT NULL,"
                "nonce VARCHAR(255) NOT NULL,"
                "value VARCHAR(255) NOT NULL,"
                "date VARCHAR(255) NOT NULL,"
                "sender VARCHAR(255) NOT NULL,"
                "receiver VARCHAR(255) NOT NULL,"
                "input_data VARCHAR(255),"
                "gas_price VARCHAR(255) NOT NULL,"
                "gas_limit VARCHAR(255) NOT NULL,"
                "gas_used VARCHAR(255) NOT NULL,"
                "status INTEGER NOT NULL"
                ")";

            sql << "INSERT INTO erc20_swap "
                "SELECT uid, ethereum_operation_uid, account_uid, type, hash, nonce, value, date, sender, receiver, input_data, gas_price, gas_limit, gas_used, status "
                "FROM erc20_operations";
            sql << "DROP TABLE erc20_operations";
            sql << "ALTER TABLE erc20_swap RENAME TO erc20_operations";
        }

        template <> void migrate<10>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE erc20_operations ADD COLUMN block_height BIGINT";
        }

        template <> void rollback<10>(soci::session& sql, api::DatabaseBackendType type) {
            // not supported in standard ways by SQLite :(
        }

        template <> void migrate<11>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "CREATE TABLE tezos_currencies("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "identifier VARCHAR(255) NOT NULL,"
                    "xpub_version VARCHAR(255) NOT NULL,"
                    "implicit_prefix VARCHAR(255) NOT NULL,"
                    "originated_prefix VARCHAR(255) NOT NULL,"
                    "message_prefix VARCHAR(255) NOT NULL,"
                    "additional_TIPs TEXT"
                    ")";

            sql << "CREATE TABLE tezos_accounts("
                    "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "idx INTEGER NOT NULL,"
                    "address VARCHAR(255) NOT NULL"
                    ")";

            sql << "CREATE TABLE tezos_transactions("
                    "transaction_uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "hash VARCHAR(255) NOT NULL,"
                    "value VARCHAR(255) NOT NULL,"
                    "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                    "time VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "receiver VARCHAR(255) NOT NULL,"
                    "fees VARCHAR(255) NOT NULL,"
                    "gas_limit VARCHAR(255) NOT NULL,"
                    "storage_limit VARCHAR(255) NOT NULL,"
                    "confirmations BIGINT NOT NULL,"
                    "type VARCHAR(255) NOT NULL,"
                    "public_key VARCHAR(255) NOT NULL,"
                    "originated_account VARCHAR(255) NOT NULL" // address:spendable:originated
                    ")";

            sql << "CREATE TABLE tezos_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "transaction_uid VARCHAR(255) NOT NULL REFERENCES tezos_transactions(transaction_uid),"
                    "transaction_hash VARCHAR(255) NOT NULL"
                    ")";

            // Originated accounts
            sql << "CREATE TABLE tezos_originated_accounts("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL ,"
                    "tezos_account_uid VARCHAR(255) NOT NULL REFERENCES tezos_accounts(uid) ON DELETE CASCADE,"
                    "address VARCHAR(255) NOT NULL,"
                    "spendable INTEGER NOT NULL,"
                    "delegatable INTEGER NOT NULL,"
                    "public_key VARCHAR(255) NOT NULL"
                    ")";

            sql << "CREATE TABLE tezos_originated_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "transaction_uid VARCHAR(255) NOT NULL REFERENCES tezos_transactions(transaction_uid),"
                    "originated_account_uid VARCHAR(255) NOT NULL REFERENCES tezos_originated_accounts(uid) ON DELETE CASCADE"
                    ")";
        }

        template <> void rollback<11>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE tezos_originated_operations";

            sql << "DROP TABLE tezos_originated_accounts";

            sql << "DROP TABLE tezos_operations";

            sql << "DROP TABLE tezos_transactions";

            sql << "DROP TABLE tezos_accounts";

            sql << "DROP TABLE tezos_currencies";
        }

        template <> void migrate<12>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "CREATE TABLE internal_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL ,"
                    "ethereum_operation_uid VARCHAR(255) NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "type VARCHAR(255) NOT NULL,"
                    "value VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "receiver VARCHAR(255) NOT NULL,"
                    "gas_limit VARCHAR(255) NOT NULL,"
                    "gas_used VARCHAR(255) NOT NULL,"
                    "input_data VARCHAR(255)"
                    ")";
        }

        template <> void rollback<12>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE internal_operations";
        }

        template <> void migrate<13>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE bitcoin_outputs ADD COLUMN block_height BIGINT";
        }

        template <> void rollback<13>(soci::session& sql, api::DatabaseBackendType type) {
        }

        template <> void migrate<14>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE ripple_transactions ADD COLUMN sequence BIGINT";
        }

        template <> void rollback<14>(soci::session& sql, api::DatabaseBackendType type) {
        }

        template <> void migrate<15>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE ripple_transactions ADD COLUMN destination_tag BIGINT";
        }

        template <> void rollback<15>(soci::session& sql, api::DatabaseBackendType type) {
        }

        template <> void migrate<16>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE tezos_accounts RENAME COLUMN address TO public_key";
        }

        template <> void rollback<16>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE tezos_accounts RENAME COLUMN public_key TO address";
        }

        template <> void migrate<17>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE tezos_transactions ADD COLUMN status BIGINT";
        }

        template <> void rollback<17>(soci::session& sql, api::DatabaseBackendType type) {
        }

        template <> void migrate<18>(soci::session& sql, api::DatabaseBackendType type) {
            // 1 if success, 0 otherwise
            // <https://xrpl.org/transaction-results.html>
            sql << "ALTER TABLE ripple_transactions ADD COLUMN status INTEGER";
        }

        template <> void rollback<18>(soci::session& sql, api::DatabaseBackendType type) {

        }

        template <> void migrate<19>(soci::session& sql,  api::DatabaseBackendType type) {
            // Stellar currencies
            sql << "CREATE TABLE stellar_currencies("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "identifier VARCHAR(255) NOT NULL,"
                    "address_version VARCHAR(255) NOT NULL,"
                    "base_reserve BIGINT NOT NULL,"
                    "base_fee BIGINT NOT NULL,"
                    "network_passphrase TEXT NOT NULL,"
                    "additional_SEPs TEXT NOT NULL"
                   ")";

            // Stellar assets
            sql << "CREATE TABLE stellar_assets("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                   "asset_type VARCHAR(255) NOT NULL,"
                   "asset_code VARCHAR(255),"
                   "asset_issuer VARCHAR(255)"
                   ")";

            // Stellar accounts
            sql << "CREATE TABLE stellar_accounts("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "idx INTEGER NOT NULL,"
                   "address TEXT NOT NULL,"
                   "subentries_count INTEGER DEFAULT 0,"
                   "sequence VARCHAR(255) DEFAULT '0'"
                   ")";

            // Stellar account signers
            sql << "CREATE TABLE stellar_account_signers("
                   "account_uid VARCHAR(255) REFERENCES stellar_accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "weight INTEGER NOT NULL,"
                   "signer_key VARCHAR(255) NOT NULL,"
                   "key_type VARCHAR(255) NOT NULL"
                   ")";

            // Stellar balances
            sql << "CREATE TABLE stellar_account_balances("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                   "account_uid VARCHAR(255) NOT NULL REFERENCES stellar_accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "asset_uid VARCHAR(255) NOT NULL REFERENCES stellar_assets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "amount VARCHAR(255) NOT NULL,"
                   "buying_liabilities VARCHAR(255),"
                   "selling_liabilities VARCHAR(255)"
                   ")";

            // Stellar transactions
            sql << "CREATE TABLE stellar_transactions("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL, "
                   "hash VARCHAR(255) NOT NULL,"
                   "source_account VARCHAR(255) NOT NULL,"
                   "sequence VARCHAR(255) NOT NULL,"
                   "fee VARCHAR(255) NOT NULL,"
                   "successful INTEGER NOT NULL,"
                   "ledger VARCHAR(255) NOT NULL,"
                   "memo_type TEXT NOT NULL,"
                   "memo TEXT NOT NULL"
                   ")";

            // Stellar native operations
            sql << "CREATE TABLE stellar_operations("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                   "transaction_uid VARCHAR(255) NOT NULL REFERENCES stellar_transactions(uid) ON DELETE CASCADE,"
                   "hash VARCHAR(255) NOT NULL,"
                   "created_at VARCHAR(255) NOT NULL,"
                   "asset_uid VARCHAR(255) NOT NULL REFERENCES stellar_assets(uid) ON DELETE CASCADE,"
                   "source_asset_uid VARCHAR(255) REFERENCES stellar_assets(uid) ON DELETE CASCADE,"
                   "amount VARCHAR(255) NOT NULL,"
                   "source_amount VARCHAR(255),"
                   "from_address VARCHAR(255) NOT NULL,"
                   "to_address VARCHAR(255) NOT NULL,"
                   "type INTEGER NOT NULL"
                   ")";


            // Stellar account operations
            sql << "CREATE TABLE stellar_account_operations("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                   "operation_uid VARCHAR(255) NOT NULL REFERENCES stellar_operations(uid) ON DELETE CASCADE"
                   ")";

            sql << "CREATE TABLE stellar_ledgers("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES blocks(uid) ON DELETE CASCADE,"
                   "base_fee VARCHAR(255) NOT NULL,"
                   "base_reserve VARCHAR(255) NOT NULL"
                   ")";

        }

        template <> void rollback<19>(soci::session& sql, api::DatabaseBackendType type) {
            // Stellar ledgers
            sql << "DROP TABLE stellar_ledgers";
            // Stellar account <> operation link table
            sql << "DROP TABLE stellar_account_operations";
            // Stellar "native" operations
            sql << "DROP TABLE stellar_operations";
            // Stellar transactions
            sql << "DROP TABLE stellar_transactions";
            // Stellar balances
            sql << "DROP TABLE stellar_account_balances";
            // Stellar account signers
            sql << "DROP TABLE stellar_account_signers";
            // Stellar accounts
            sql << "DROP TABLE stellar_accounts";
            // Stellar assets
            sql << "DROP TABLE stellar_assets";
            // Stellar currencies
            sql << "DROP TABLE stellar_currencies";
        }

        template <> void migrate<20>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "CREATE TABLE cosmos_currencies("
                "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                "identifier VARCHAR(255) NOT NULL,"
                "xpub_version VARCHAR(255) NOT NULL,"
                "pubkey_prefix VARCHAR(255) NOT NULL,"
                "address_prefix VARCHAR(255) NOT NULL,"
                "message_prefix VARCHAR(255) NOT NULL,"
                "chain_id VARCHAR(255) NOT NULL,"
                "additional_CIPs TEXT"
                ")";

            sql << "CREATE TABLE cosmos_accounts("
                "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "idx INTEGER NOT NULL,"
                "pubkey VARCHAR(255) NOT NULL,"
                "account_type VARCHAR(255),"
                "account_number VARCHAR(255),"
                "sequence VARCHAR(255),"
                "balances VARCHAR(255),"
                "withdraw_address VARCHAR(255),"
                "last_update TEXT"
                ")";

            sql << "CREATE TABLE cosmos_transactions("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "hash VARCHAR(255) NOT NULL,"
                "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                "time VARCHAR(255) NOT NULL,"
                "fee_amount VARCHAR(255) NOT NULL,"
                "gas VARCHAR(255) NOT NULL,"
                "gas_used VARCHAR(255),"
                "memo TEXT"
                ")";

            // * NOTE : Special Msg types
            // ** MsgMultiSend
            // MsgMultiSend can have an arbitrary number of inputs and outputs
            // (only invariant is sum(inputs) == sum(outputs))
            // Therefore the various inputs and outputs are stored in another
            // table to avoid varchars longer than 256 bytes
            //
            // ** MsgCreateValidator
            // This message is not handled in database, and behaviour with these
            // messages is undefined.
            //
            // ** MsgEditValidator
            // This message is not handled in database, and behaviour with these
            // messages is undefined.
            sql << "CREATE TABLE cosmos_messages("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "transaction_uid VARCHAR(255) NOT NULL "
                    "REFERENCES cosmos_transactions(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "message_type VARCHAR(255) NOT NULL,"
                "log TEXT,"
                "success INTEGER,"
                "msg_index INTEGER NOT NULL,"
                // MsgSend
                "from_address VARCHAR(255),"
                "to_address VARCHAR(255),"
                "amount VARCHAR(255),"
                // MsgDelegate & MsgUndelegate
                "delegator_address VARCHAR(255),"
                "validator_address VARCHAR(255),"
                // MsgBeginRedelegate
                "validator_src_address VARCHAR(255),"
                "validator_dst_address VARCHAR(255),"
                // MsgSubmitProposal
                "content_type TEXT,"
                "content_title TEXT,"
                "content_description TEXT,"
                "proposer VARCHAR(255),"
                // MsgVote
                "voter VARCHAR(255),"
                "proposal_id VARCHAR(255),"
                "vote_option VARCHAR(255),"
                // MsgDeposit
                "depositor VARCHAR(255)"
                ")";

            sql << "CREATE TABLE cosmos_multisend_io("
                "message_uid VARCHAR(255) NOT NULL "
                "REFERENCES cosmos_messages(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                // not null when input
                "from_address VARCHAR(255),"
                // not null when output
                "to_address VARCHAR(255),"
                "amount VARCHAR(255) NOT NULL"
                ")";

            sql << "CREATE TABLE cosmos_operations("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL "
                "REFERENCES operations(uid) ON DELETE CASCADE,"
                "message_uid VARCHAR(255) NOT NULL "
                "REFERENCES cosmos_messages(uid) ON DELETE CASCADE ON UPDATE CASCADE"
                ")";
        }

        template <> void rollback<20>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE cosmos_multisend_io";
            sql << "DROP TABLE cosmos_operations";
            sql << "DROP TABLE cosmos_messages";
            sql << "DROP TABLE cosmos_transactions";
            sql << "DROP TABLE cosmos_accounts";
            sql << "DROP TABLE cosmos_currencies";

        }


        template <> void migrate<21>(soci::session& sql, api::DatabaseBackendType type) {
           sql << "ALTER TABLE bitcoin_outputs ADD replaceable INTEGER DEFAULT 0";
        }

        template <> void rollback<21>(soci::session& sql, api::DatabaseBackendType type) {
            // SQLite doesn't handle ALTER TABLE DROP
            if (type != api::DatabaseBackendType::SQLITE3) {
                sql << "ALTER TABLE bitcoin_outputs DROP replaceable";
            } else {
                sql << "CREATE TABLE bitcoin_outputs_swap("
                       "idx INTEGER NOT NULL,"
                       "transaction_uid VARCHAR(255) NOT NULL REFERENCES bitcoin_transactions(transaction_uid) ON DELETE CASCADE,"
                       "transaction_hash VARCHAR(255) NOT NULL,"
                       "amount BIGINT NOT NULL,"
                       "script TEXT NOT NULL,"
                       "address VARCHAR(255),"
                       "account_uid VARCHAR(255),"
                       "PRIMARY KEY (idx, transaction_uid)"
                       ")";
                sql << "INSERT INTO bitcoin_outputs_swap "
                       "SELECT idx, transaction_uid, transaction_hash, amount, script, "
                                   "address, account_uid "
                       "FROM bitcoin_outputs";
                sql << "DROP TABLE bitcoin_outputs";
                sql << "ALTER TABLE bitcoin_outputs_swap RENAME TO bitcoin_outputs";
            }
        }

        template <> void migrate<22>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE stellar_currencies ADD muxed_address_version VARCHAR(255)";
            sql << "UPDATE stellar_currencies SET muxed_address_version = '60'";
        }

        template <> void rollback<22>(soci::session& sql, api::DatabaseBackendType type) {
            // SQLite doesn't handle ALTER TABLE DROP
            if (type != api::DatabaseBackendType::SQLITE3) {
                sql << "ALTER TABLE stellar_currencies DROP muxed_address_version";
            } else {
                sql << "CREATE TABLE stellar_currencies_swap("
                       "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                       "identifier VARCHAR(255) NOT NULL,"
                       "address_version VARCHAR(255) NOT NULL,"
                       "base_reserve BIGINT NOT NULL,"
                       "base_fee BIGINT NOT NULL,"
                       "network_passphrase TEXT NOT NULL,"
                       "additional_SEPs TEXT NOT NULL"
                       ")";
                sql << "INSERT INTO stellar_currencies_swap "
                       "SELECT name, identifier, address_version, base_reserve, base_fee, "
                       "network_passphrase, additional_SEPs "
                       "FROM stellar_currencies";
                sql << "DROP TABLE stellar_currencies";
                sql << "ALTER TABLE stellar_currencies_swap RENAME TO stellar_currencies";
            }
        }

        template <> void migrate<23>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "CREATE TABLE algorand_currencies("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "genesis_id VARCHAR(255) NOT NULL,"
                    "genesis_hash VARCHAR(255) NOT NULL"
                   ")";

            sql << "CREATE TABLE algorand_accounts("
                    "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "idx INTEGER NOT NULL,"
                    "address VARCHAR(255) NOT NULL"
                    ")";

            sql << "CREATE TABLE algorand_transactions("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                    "hash VARCHAR(255),"
                    "type VARCHAR(255) NOT NULL,"
                    "round BIGINT,"
                    "timestamp BIGINT,"
                    "first_valid BIGINT NOT NULL,"
                    "last_valid BIGINT NOT NULL,"
                    "genesis_id VARCHAR(255),"
                    "genesis_hash VARCHAR(255) NOT NULL,"
                    "sender VARCHAR(255) NOT NULL,"
                    "fee BIGINT NOT NULL,"
                    "note VARCHAR(255),"
                    "groupVal VARCHAR(255),"
                    "leaseVal VARCHAR(255),"
                    "sender_rewards BIGINT,"
                    "receiver_rewards BIGINT,"
                    "close_rewards BIGINT,"

                    // Fields for payment transactions
                    "pay_amount BIGINT,"
                    "pay_receiver_address VARCHAR(255),"
                    "pay_close_address VARCHAR(255),"
                    "pay_close_amount BIGINT,"

                    // Fields for key registration transactions
                    "keyreg_non_participation INTEGER,"
                    "keyreg_selection_pk VARCHAR(255),"
                    "keyreg_vote_pk VARCHAR(255),"
                    "keyreg_vote_key_dilution BIGINT,"
                    "keyreg_vote_first BIGINT,"
                    "keyreg_vote_last BIGINT,"

                    // Fields for asset config transactions
                    "acfg_asset_id BIGINT,"
                    "acfg_asset_name VARCHAR(255),"
                    "acfg_unit_name VARCHAR(255),"
                    "acfg_total BIGINT,"
                    "acfg_decimals INTEGER,"
                    "acfg_default_frozen INTEGER,"
                    "acfg_creator_address VARCHAR(255),"
                    "acfg_manager_address VARCHAR(255),"
                    "acfg_reserve_address VARCHAR(255),"
                    "acfg_freeze_address VARCHAR(255),"
                    "acfg_clawback_address VARCHAR(255),"
                    "acfg_metadata_hash VARCHAR(255),"
                    "acfg_url VARCHAR(255),"

                    // Fields for asset transfer transactions
                    "axfer_asset_id BIGINT,"
                    "axfer_asset_amount BIGINT,"
                    "axfer_receiver_address VARCHAR(255),"
                    "axfer_close_address VARCHAR(255),"
                    "axfer_close_amount BIGINT,"
                    "axfer_sender_address VARCHAR(255),"

                    // Fields for asset freeze transactions
                    "afrz_asset_id BIGINT,"
                    "afrz_frozen INTEGER,"
                    "afrz_frozen_address VARCHAR(255)"
                    ")";

            sql << "CREATE TABLE algorand_operations("
                    "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                    "transaction_uid VARCHAR(255) NOT NULL REFERENCES algorand_transactions(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "transaction_hash VARCHAR(255) NOT NULL"
                    ")";
        }

        template <> void rollback<23>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "DROP TABLE algorand_operations";

            sql << "DROP TABLE algorand_transactions";

            sql << "DROP TABLE algorand_accounts";

            sql << "DROP TABLE algorand_currencies";
        }

        template <> void migrate<24>(soci::session& sql, api::DatabaseBackendType type) {
           sql << "ALTER TABLE bitcoin_outputs ALTER COLUMN address TYPE TEXT";
        }

        template <> void rollback<24>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE bitcoin_outputs ALTER COLUMN address TYPE VARCHAR(255)";
        }

    }
}
