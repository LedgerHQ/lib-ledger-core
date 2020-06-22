#pragma once

#include <core/api/DatabaseBackendType.hpp>
#include <core/database/Migrations.hpp>

namespace ledger {
namespace core {
/// Tag type.
struct CosmosMigration {
  static int constexpr COIN_ID = 118;
  static uint32_t constexpr CURRENT_VERSION = 1;
};

// migrations
template <>
void migrate<1, CosmosMigration>(soci::session &sql,
                                 api::DatabaseBackendType type);
template <>
void rollback<1, CosmosMigration>(soci::session &sql,
                                  api::DatabaseBackendType type);
} // namespace core
} // namespace ledger
