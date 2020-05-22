#include <algorand/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr AlgorandMigration::COIN_ID;
        uint32_t constexpr AlgorandMigration::CURRENT_VERSION;

        template <> void migrate<1, AlgorandMigration>(soci::session& sql, api::DatabaseBackendType type) {
        }

        template <> void rollback<1, AlgorandMigration>(soci::session& sql, api::DatabaseBackendType type) {
        }
    }
}
