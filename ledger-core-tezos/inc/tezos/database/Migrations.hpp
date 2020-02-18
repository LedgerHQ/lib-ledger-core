#pragma once

#include <core/api/DatabaseBackendType.hpp>
#include <core/database/Migrations.hpp>

#include <tezos/TezosLikeCoinID.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct TezosMigration {
          static int constexpr COIN_ID = TEZOS_COIN_ID; 
          static uint32_t constexpr CURRENT_VERSION = 3; 
        };

        // migrations
        template <> void migrate<1, TezosMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<1, TezosMigration>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<2, TezosMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<2, TezosMigration>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<3, TezosMigration>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<3, TezosMigration>(soci::session& sql, api::DatabaseBackendType type);
  }
}
