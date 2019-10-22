#include <$project_name/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr $project_nameMigration::coinID;
        uint32_t constexpr $project_nameMigration::currentVersion;

        template <> void migrate<1, $project_nameMigration>(soci::session& sql) {
        }

        template <> void rollback<1, $project_nameMigration>(soci::session& sql) {
        }
    }
}
