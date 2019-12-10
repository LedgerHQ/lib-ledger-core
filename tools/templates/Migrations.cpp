#include <$project_name/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr $project_nameMigration::COIN_ID;
        uint32_t constexpr $project_nameMigration::CURRENT_VERSION;

        template <> void migrate<1, $project_nameMigration>(soci::session& sql) {
        }

        template <> void rollback<1, $project_nameMigration>(soci::session& sql) {
        }
    }
}
