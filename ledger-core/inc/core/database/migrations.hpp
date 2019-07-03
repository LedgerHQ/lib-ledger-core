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

#pragma once

#include <soci.h>
#include <iostream>

#include <core/utils/Exception.hpp>

namespace ledger {
    namespace core {
        /// Get the current database migration version.
        ///
        /// The coinID trait is required on T.
        template <typename T>
        int getDatabaseMigrationVersion2(soci::session& sql) {
            int version = -1;

            try {
                soci::statement st = (sql.prepare << "SELECT version FROM __database_meta__ WHERE id = :id", soci::use(T::coinID), soci::into(version));
                st.execute();
                st.fetch();
            } catch (...) {
                // if we cannot find the version, it stays set to -1
            }

            return version;
        }

        /// Get the current database migration version.
        int getDatabaseMigrationVersion(soci::session& sql);

        template <typename T>
        int getDatabaseMigrationVersion2(soci::session& sql, T& migrationSystem) {
            int version = -1;
            auto coinID = migrationSystem.getCoinID();

            try {
                soci::statement st = (sql.prepare << "SELECT version FROM __database_meta__ WHERE id = :coin_id", soci::use(coinID), soci::into(version));
                st.execute();
                st.fetch();
            } catch (...) {
                // if we cannot find the version, it stays set to -1
            }

            return version;
        }

        template <int migrationNumber>
        void migrate(soci::session& sql) {
            std::cerr << "No specified migration for version " << migrationNumber << std::endl;
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "No specified migration for version {}", migrationNumber);
        }

        template <int version>
        bool migrate(soci::session& sql, int currentVersion) {
            bool previousResult = migrate<version - 1>(sql, currentVersion);

            if (currentVersion < version) {
                migrate<version>(sql);
                sql << "UPDATE __database_meta__ SET version = :version", soci::use(version);
                return true;
            }

            return previousResult;
        };

        template <int version, typename T>
        void migrate2(soci::session& sql) {
            std::cerr << "No specified migration for version " << version << std::endl;
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "No specified migration for version {}", version);
        }

        template <int version, typename T>
        bool migrate2(soci::session& sql, int currentVersion) {
            bool previousResult = migrate2<version - 1, T>(sql, currentVersion);

            if (currentVersion < version) {
                migrate2<version, T>(sql);
                sql << "UPDATE __database_meta__ SET id = :id, version = :version", soci::use(T::coinID), soci::use(version);

                return true;
            }

            return previousResult;
        };

        template <int version>
        void rollback(soci::session& sql) {
            //std::cerr << "No specified rollback for version " << version << std::endl;
            //throw make_exception(api::ErrorCode::RUNTIME_ERROR, "No specified rollback for version {}", version);
        }

        /// Rollback all migrations down.
        ///
        /// This is a bit like dropping the content of the database, but does it in a more correct
        /// and portable way. Also, it enables possible partial rollbacks, even though the current
        /// implementation doesn’t.
        template <int version>
        void rollback(soci::session& sql, int currentVersion) {
            if (currentVersion == version) {
                // we’re in sync with the database; perform the rollback normally
                rollback<version>(sql);

                if (version >= 0) {
                    // after rolling back this migration, we won’t have anything left, so we only
                    // update the version for > 0
                    if (version != 0) {
                        auto prevVersion = version - 1;
                        sql << "UPDATE __database_meta__ SET version = :version", soci::use(prevVersion);
                    }

                    rollback<version - 1>(sql, currentVersion - 1);
                }
            } else if (currentVersion < version) {
                // we’re trying to rollback a migration that hasn’t been applied; try the previous
                // rollback
                rollback<version - 1>(sql, currentVersion);
            } else {
                // we’re trying to rollback a migration but we have missed some others; apply the
                // next ones first
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Missing rollback migrations: {} to {}", version + 1, currentVersion);
            }
        }

        template <int version, typename T>
        void rollback2(soci::session& sql) {
            //std::cerr << "No specified rollback for version " << version << std::endl;
            //throw make_exception(api::ErrorCode::RUNTIME_ERROR, "No specified rollback for version {}", version);
        }

        /// Rollback all migrations down.
        ///
        /// This is a bit like dropping the content of the database, but does it in a more correct
        /// and portable way. Also, it enables possible partial rollbacks, even though the current
        /// implementation doesn’t.
        template <int version, typename T>
        void rollback2(soci::session& sql, int currentVersion) {
            if (currentVersion == version) {
                // we’re in sync with the database; perform the rollback normally
                rollback2<version, T>(sql);

                if (version >= 0) {
                    // after rolling back this migration, we won’t have anything left, so we only
                    // update the version for > 0
                    if (version != 0) {
                        auto prevVersion = version - 1;
                        sql << "UPDATE __database_meta__ SET id = :id, version = :version", soci::use(T::coinID), soci::use(prevVersion);
                    }

                    rollback2<version - 1, T>(sql, currentVersion - 1);
                }
            } else if (currentVersion < version) {
                // we’re trying to rollback a migration that hasn’t been applied; try the previous
                // rollback
                rollback2<version - 1, T>(sql, currentVersion);
            } else {
                // we’re trying to rollback a migration but we have missed some others; apply the
                // next ones first
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Missing rollback migrations: {} to {}", version + 1, currentVersion);
            }
        }

        /// Migration system.
        ///
        /// The T type variable is expected to be a trait that has some constants:
        ///
        ///   - T::coinID: coin ID numeric value. Keep in mind that this value must be unique for
        ///     all coins as it serves as a namespace value.
        ///
        ///     See <https://github.com/satoshilabs/slips/blob/master/slip-0044.md> to correctly
        ///     implement this function.
        template <int version, typename T>
        struct Migration final {
          /// Advance the migration system up to migrationNumber.
          static void migrate(soci::session& sql, int currentVersion) {
              Migration<version - 1, T>::migrate(sql, currentVersion);

              if (currentVersion < version) {
                  migrate2<version, T>(sql);
                  sql << "UPDATE __database_meta__ SET id = :id, version = :version", soci::use(T::coinID), soci::use(version);
              }
          }

          /// Rollback from migrationNumber.
          static void rollback(soci::session& sql, int currentVersion) {
              rollback2<version, T>(sql, currentVersion);
          }
        };

        template <typename T>
        struct Migration<0, T> final {
           static void migrate(soci::session& sql, int currentVersion) {
               sql << "INSERT INTO __database_meta__(id, version) VALUES(:id, 0)", soci::use(T::coinID);
           }

           static void rollback(soci::session& sql, int currentVersion) {
             sql << "DELETE FROM __database_meta__ where id = :id", soci::use(T::coinID);
           }
        };

        template <> bool migrate<-1>(soci::session& sql, int currentVersion);
        template <> void rollback<-1>(soci::session& sql, int currentVersion);

        // default migrations

        /// Init migrations system.
        void setupMigrations(soci::session& sql);

        /// De-init migrations system.
        void unsetupMigrations(soci::session& sql);

        /// The Core tag type.
        struct CoreMigration {
            static int const coinID;
        };

        // migrations
        template <> void migrate2<1, CoreMigration>(soci::session& sql);
        template <> void rollback2<1, CoreMigration>(soci::session& sql);

        template <> void migrate<0>(soci::session& sql);
        template <> void rollback<0>(soci::session& sql);

        template <> void migrate<1>(soci::session& sql);
        template <> void rollback<1>(soci::session& sql);

        template <> void migrate<2>(soci::session& sql);
        template <> void rollback<2>(soci::session& sql);

        template <> void migrate<3>(soci::session& sql);
        template <> void rollback<3>(soci::session& sql);

        template <> void migrate<4>(soci::session& sql);
        template <> void rollback<4>(soci::session& sql);

        template <> void migrate<5>(soci::session& sql);
        template <> void rollback<5>(soci::session& sql);

        template <> void migrate<6>(soci::session& sql);
        template <> void rollback<6>(soci::session& sql);

        template <> void migrate<7>(soci::session& sql);
        template <> void rollback<7>(soci::session& sql);

        // add ripple’s memo field
        template <> void migrate<8>(soci::session& sql);
        template <> void rollback<8>(soci::session& sql);
    }
}
