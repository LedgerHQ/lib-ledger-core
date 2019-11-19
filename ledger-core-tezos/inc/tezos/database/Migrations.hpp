#pragma once

#include <tezos/TezosLikeCoinID.hpp>

#include <core/database/Migrations.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct TezosMigration {
          static int constexpr COIN_ID = TEZOS_COIN_ID; 
          static uint32_t constexpr CURRENT_VERSION = 2; 
        };

        // migrations
        template <> void migrate<1, TezosMigration>(soci::session& sql);
        template <> void rollback<1, TezosMigration>(soci::session& sql);

        template <> void migrate<2, TezosMigration>(soci::session& sql);
        template <> void rollback<2, TezosMigration>(soci::session& sql);
  }
}
