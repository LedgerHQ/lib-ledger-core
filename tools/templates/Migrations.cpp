#include <$project_name/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int const $project_nameMigration::coinID = 9999999; // TODO: edit

        uint32_t const $project_nameMigration::currentVersion = 1;

        template <> void migrate<1, $project_nameMigration>(soci::session& sql) {
        }

        template <> void rollback<1, $project_nameMigration>(soci::session& sql) {
        }
    }
}
