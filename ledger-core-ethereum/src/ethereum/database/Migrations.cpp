#include <ethereum/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int const ethereumMigration::coinID = 60;

        uint32_t const ethereumMigration::currentVersion = 1;

        template <> void migrate<1, ethereumMigration>(soci::session& sql) {
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

        template <> void rollback<1, ethereumMigration>(soci::session& sql) {
            // ERC20 tokens
            sql << "DROP TABLE erc20_tokens";

            // ERC20 operations
            sql << "DROP TABLE erc20_operations";

            // ERC20 accounts
            sql << "DROP TABLE erc20_accounts";

            // ETH operations
            sql << "DROP TABLE ethereum_operations";

            // ETH transactions
            sql << "DROP TABLE ethereum_transactions";

            // ETH accounts
            sql << "DROP TABLE ethereum_accounts";

            // ETH currencies
            sql << "DROP TABLE ethereum_currencies";
        }
    }
}
