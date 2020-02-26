#pragma once

#include <core/api/DatabaseBackendType.hpp>
#include <core/database/Migrations.hpp>

#include <bitcoin/BitcoinLikeCoinID.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct BitcoinMigration {
          static int constexpr COIN_ID = BITCOIN_COIN_ID;
          static uint32_t constexpr CURRENT_VERSION = 6;
        };

        // migrations
        template <> void migrate<1, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<1, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<2, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<2, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<3, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<3, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<4, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<4, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<5, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<5, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<6, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<6, BitcoinMigration>(soci::session& sql, api::DatabaseBackendType type);
  }
}
