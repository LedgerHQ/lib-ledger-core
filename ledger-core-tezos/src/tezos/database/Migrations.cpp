#include <tezos/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr TezosMigration::COIN_ID;
        uint32_t constexpr TezosMigration::CURRENT_VERSION;

        template <> void migrate<1, TezosMigration>(soci::session& sql) {
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

        template <> void rollback<1, TezosMigration>(soci::session& sql) {
            sql << "DROP TABLE tezos_originated_operations";

            sql << "DROP TABLE tezos_originated_accounts";

            sql << "DROP TABLE tezos_operations";

            sql << "DROP TABLE tezos_transactions";

            sql << "DROP TABLE tezos_accounts";

            sql << "DROP TABLE tezos_currencies";
        }


        template <> void migrate<2, TezosMigration>(soci::session& sql) {
            sql << "ALTER TABLE tezos_accounts RENAME COLUMN address TO public_key";
        }

        template <> void rollback<2, TezosMigration>(soci::session& sql) {
            sql << "ALTER TABLE tezos_accounts RENAME COLUMN public_key TO address";
        }

    }
}
