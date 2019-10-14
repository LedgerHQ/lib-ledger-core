#pragma once

#include <core/database/Migrations.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct $project_nameMigration {
          static int const coinID;
          static uint32_t const currentVersion;
        };

        // migrations
        template <> void migrate<1, $project_nameMigration>(soci::session& sql);
        template <> void rollback<1, $project_nameMigration>(soci::session& sql);
  }
}
