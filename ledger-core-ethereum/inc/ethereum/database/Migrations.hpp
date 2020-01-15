#pragma once

#include <core/database/Migrations.hpp>
#include <ethereum/EthereumLikeCoinID.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct EthereumMigration {
          static int constexpr COIN_ID = ETHEREUM_COIN_ID;
          static uint32_t constexpr CURRENT_VERSION = 1;
        };

        // migrations
        template <> void migrate<1, EthereumMigration>(soci::session& sql);
        template <> void rollback<1, EthereumMigration>(soci::session& sql);
  }
}
