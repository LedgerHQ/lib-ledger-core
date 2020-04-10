#pragma once

#include <core/database/Migrations.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct AlgorandMigration {
          static int constexpr COIN_ID = 283;
          static uint32_t constexpr CURRENT_VERSION = 1;
        };

        // migrations
        template <> void migrate<1, AlgorandMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<1, AlgorandMigration>(soci::session& sql, api::DatabaseBackendType type);
  }
}
