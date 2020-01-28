#include <ethereum/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr EthereumMigration::COIN_ID;

        uint32_t constexpr EthereumMigration::CURRENT_VERSION;

        template <> void migrate<1, EthereumMigration>(soci::session& sql) {
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

        template <> void rollback<1, EthereumMigration>(soci::session& sql) {
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

        template <> void migrate<2, EthereumMigration>(soci::session& sql) {
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
            sql << "DROP TABLE ethereum_transactions";
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

        template <> void rollback<2, EthereumMigration>(soci::session& sql) {
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
            sql << "DROP TABLE ethereum_transactions";
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

        template <> void migrate<3, EthereumMigration>(soci::session& sql) {
            sql << "ALTER TABLE erc20_operations ADD COLUMN block_height BIGINT";
        }

        template <> void rollback<3, EthereumMigration>(soci::session& sql) {
        }
    }
}
