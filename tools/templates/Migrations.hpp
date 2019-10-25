#pragma once

#include <core/database/Migrations.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct $project_nameMigration {
          static int constexpr coinID = 99999; // TODO: edit
          static uint32_t constexpr currentVersion = 1; // TODO: edit
        };

        // migrations
        template <> void migrate<1, $project_nameMigration>(soci::session& sql);
        template <> void rollback<1, $project_nameMigration>(soci::session& sql);
  }
}
