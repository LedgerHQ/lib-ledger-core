/*
 *
 * migrations
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/04/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef LEDGER_CORE_MIGRATIONS_HPP
#define LEDGER_CORE_MIGRATIONS_HPP

#include <soci.h>
#include <iostream>
#include <api/DatabaseBackendType.hpp>
#include "utils/Exception.hpp"

namespace ledger {
    namespace core {
        /// Maximum length of stored VARCHAR
        static const int MAX_LENGTH_VAR_CHAR = 255;

        /// Get the current database migration version.
        int getDatabaseMigrationVersion(soci::session& sql);

        template <int migrationNumber>
        void migrate(soci::session& sql, api::DatabaseBackendType type) {
            std::cerr << "No specified migration for version " << migrationNumber << std::endl;
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "No specified migration for version {}", migrationNumber);
        }

        template <int version>
        bool migrate(soci::session& sql, int currentVersion, api::DatabaseBackendType type) {
            bool previousResult = migrate<version - 1>(sql, currentVersion, type);

            if (currentVersion < version) {
                migrate<version>(sql, type);
                sql << "UPDATE __database_meta__ SET version = :version", soci::use(version);
                return true;
            }

            return previousResult;
        };

        template <int version>
        void rollback(soci::session& sql, api::DatabaseBackendType type) {
            //std::cerr << "No specified rollback for version " << version << std::endl;
            //throw make_exception(api::ErrorCode::RUNTIME_ERROR, "No specified rollback for version {}", version);
        }

        /// Rollback all migrations down.
        ///
        /// This is a bit like dropping the content of the database, but does it in a more correct
        /// and portable way. Also, it enables possible partial rollbacks, even though the current
        /// implementation doesn’t.
        template <int version>
        void rollback(soci::session& sql, int currentVersion, api::DatabaseBackendType type) {
            if (currentVersion == version) {
                // we’re in sync with the database; perform the rollback normally
                rollback<version>(sql, type);

                if (version >= 0) {
                    // after rolling back this migration, we won’t have anything left, so we only
                    // update the version for > 0
                    if (version != 0) {
                        auto prevVersion = version - 1;
                        sql << "UPDATE __database_meta__ SET version = :version", soci::use(prevVersion);
                    }

                    rollback<version - 1>(sql, currentVersion - 1, type);
                }
            } else if (currentVersion < version) {
                // we’re trying to rollback a migration that hasn’t been applied; try the previous
                // rollback
                rollback<version - 1>(sql, currentVersion, type);
            } else {
                // we’re trying to rollback a migration but we have missed some others; apply the
                // next ones first
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Missing rollback migrations: {} to {}", version + 1, currentVersion);
            }
        }

        template <> bool migrate<-1>(soci::session& sql, int currentVersion, api::DatabaseBackendType type);
        template <> void rollback<-1>(soci::session& sql, int currentVersion, api::DatabaseBackendType type);

        // Migrations
        template <> void migrate<0>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<0>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<1>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<1>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<2>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<2>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<3>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<3>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<4>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<4>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<5>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<5>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<6>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<6>(soci::session& sql, api::DatabaseBackendType type);

        template <> void migrate<7>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<7>(soci::session& sql, api::DatabaseBackendType type);

        // add ripple’s memo field
        template <> void migrate<8>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<8>(soci::session& sql, api::DatabaseBackendType type);

        // Migrate input_data from VARCHAR(255) to TEXT
        template <> void migrate<9>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<9>(soci::session& sql, api::DatabaseBackendType type);

        // Add block_height column to erc20_operations table
        template <> void migrate<10>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<10>(soci::session& sql, api::DatabaseBackendType type);

        // Tezos Support
        template <> void migrate<11>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<11>(soci::session& sql, api::DatabaseBackendType type);

        // Add internal transactions for ETH
        template <> void migrate<12>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<12>(soci::session& sql, api::DatabaseBackendType type);

        // Add block height to outputs
        template <> void migrate<13>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<13>(soci::session& sql, api::DatabaseBackendType type);

        // Add XRP sequence
        template <> void migrate<14>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<14>(soci::session& sql, api::DatabaseBackendType type);

        // Add XRP destination_tag
        template <> void migrate<15>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<15>(soci::session& sql, api::DatabaseBackendType type);

        // Replace XTZ address column by public_key one
        template <> void migrate<16>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<16>(soci::session& sql, api::DatabaseBackendType type);

        // Add status column on XTZ transactions
        template <> void migrate<17>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<17>(soci::session& sql, api::DatabaseBackendType type);

        // Add XRP transaction status
        template <> void migrate<18>(soci::session& sql, api::DatabaseBackendType type);
        template <> void rollback<18>(soci::session& sql, api::DatabaseBackendType type);
    }
}

#endif //LEDGER_CORE_MIGRATIONS_HPP
