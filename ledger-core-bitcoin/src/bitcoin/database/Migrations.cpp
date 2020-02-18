#include <bitcoin/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr BitcoinMigration::COIN_ID;
        uint32_t constexpr BitcoinMigration::CURRENT_VERSION;

        template <> void migrate<1, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
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

        template <> void rollback<1, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
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
        }

        template <> void migrate<2, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN timestamp_delay BIGINT DEFAULT 0";
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN sighash_type VARCHAR(255) DEFAULT 01";
        }

        template <> void rollback<2, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            // not supported in standard ways by SQLite :(
        }

        template <> void migrate<3, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            sql << "ALTER TABLE bitcoin_currencies ADD COLUMN additional_BIPs TEXT DEFAULT ''";
        }

        template <> void rollback<3, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            // not supported in standard ways by SQLite :(
        }

        template <> void migrate<4, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            auto count = 0;
            sql << "SELECT COUNT(*) FROM bitcoin_currencies WHERE identifier = 'dgb'", soci::into(count);
            if (count > 0) {
                sql << "UPDATE bitcoin_currencies SET p2sh_version = '3f' WHERE identifier = 'dgb' ";
            }
        }

        template <> void rollback<4, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            // cannot rollback
        }

        template <> void migrate<5, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            sql << "CREATE TABLE bech32_parameters("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES bitcoin_currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "hrp VARCHAR(255) NOT NULL,"
                    "separator VARCHAR(255) NOT NULL,"
                    "generator VARCHAR(255) NOT NULL,"
                    "p2wpkh_version VARCHAR(255) NOT NULL,"
                    "p2wsh_version VARCHAR(255) NOT NULL"
                    ")";
        }

        template <> void rollback<5, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            sql << "DROP TABLE bech32_parameters";
        }

        template <> void migrate<6, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
            sql << "ALTER TABLE bitcoin_outputs ADD COLUMN block_height BIGINT";
        }

        template <> void rollback<6, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType) {
        }
    }
}
