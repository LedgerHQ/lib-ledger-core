#include <algorand/database/Migrations.hpp>

namespace ledger {
namespace core {

    int constexpr AlgorandMigration::COIN_ID;
    uint32_t constexpr AlgorandMigration::CURRENT_VERSION;

    template <> void migrate<1, AlgorandMigration>(soci::session& sql, api::DatabaseBackendType type) {

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

                // Fields for payment transactions
                "pay_amount BIGINT,"
                "from_rewards BIGINT,"
                "pay_receiver_address VARCHAR(255),"
                "pay_receiver_rewards BIGINT,"
                "pay_close_address VARCHAR(255),"
                "pay_close_amount BIGINT,"
                "pay_close_rewards BIGINT,"

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
                "axfer_sender_address VARCHAR(255),"

                // Fields for asset freeze transactions
                "afrz_asset_id BIGINT,"
                "afrz_frozen INTEGER,"
                "afrz_frozen_address VARCHAR(255)"
                ")";

        sql << "CREATE TABLE algorand_asset_amounts("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "account_uid VARCHAR(255) NOT NULL "
                    "REFERENCES algorand_account(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "creator_address VARCHAR(255) NOT NULL,"
                "amount BIGINT NOT NULL,"
                "frozen INTEGER NOT NULL"
        ")";

        sql << "CREATE TABLE algorand_asset_params("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "account_uid VARCHAR(255) NOT NULL "
                    "REFERENCES algorand_account(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "asset_id BIGINT,"
                "asset_name VARCHAR(255),"
                "unit_name VARCHAR(255),"
                "total BIGINT,"
                "decimals INTEGER,"
                "frozen INTEGER,"
                "creator_address VARCHAR(255),"
                "manager_address VARCHAR(255),"
                "reserve_address VARCHAR(255),"
                "freeze_address VARCHAR(255),"
                "clawback_address VARCHAR(255),"
                "metadata_hash VARCHAR(255),"
                "url VARCHAR(255)"
        ")";

        sql << "CREATE TABLE algorand_operations("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                "transaction_uid VARCHAR(255) NOT NULL REFERENCES algorand_transactions(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "transaction_hash VARCHAR(255) NOT NULL"
        ")";
    }

    template <> void rollback<1, AlgorandMigration>(soci::session& sql, api::DatabaseBackendType type) {
        sql << "DROP TABLE algorand_operations";

        sql << "DROP TABLE algorand_asset_params";

        sql << "DROP TABLE algorand_asset_amounts";

        sql << "DROP TABLE algorand_transactions";

        sql << "DROP TABLE algorand_accounts";
    }

}
}
