#pragma once

#include <core/database/Migrations.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct EthereumMigration {
          static int const coinID;
          static uint32_t const currentVersion;
        };

        // migrations
        template <> void migrate<1, EthereumMigration>(soci::session& sql);
        template <> void rollback<1, EthereumMigration>(soci::session& sql);
  }
}
