#pragma once

#include <core/database/Migrations.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct ethereumMigration {
          static int const coinID;
          static uint32_t const currentVersion;
        };

        // migrations
        template <> void migrate<1, ethereumMigration>(soci::session& sql);
        template <> void rollback<1, ethereumMigration>(soci::session& sql);
  }
}
