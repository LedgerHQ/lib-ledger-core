/*
 *
 * DatabaseSessionPool
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/04/2017.
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

#include <core/api/ExecutionContext.hpp>
#include <core/api/DatabaseBackendType.hpp>
#include <core/async/Future.hpp>
#include <core/database/DatabaseBackend.hpp>
#include <core/database/Migrations.hpp>
#include <core/debug/LoggerStreamBuffer.hpp>

namespace ledger {
    namespace core {
        class DatabaseSessionPool {
        public:
            DatabaseSessionPool(const std::shared_ptr<DatabaseBackend> &backend,
                                const std::shared_ptr<api::PathResolver> &resolver,
                                const std::shared_ptr<spdlog::logger> &logger,
                                const std::string &dbName,
                                const std::string &password);
            soci::connection_pool& getPool();
            ~DatabaseSessionPool();

            static FuturePtr<DatabaseSessionPool> getSessionPool(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<DatabaseBackend> &backend,
                const std::shared_ptr<api::PathResolver> &resolver,
                const std::shared_ptr<spdlog::logger> &logger,
                const std::string &dbName,
                const std::string &password = ""
            );

            /// Install the required data / schemas in the database to allow coins to register.
            void performDatabaseMigrationSetup();

            /// Uninstall the data / schemas from the database that allow coins to register.
            void performDatabaseMigrationUnsetup();

            /// Run migration forward for a given migration system.
            template <typename T>
            void forwardMigration() {
                soci::session sql(getPool());
                uint32_t const version = getDatabaseMigrationVersion<T>(sql);

                soci::transaction tr(sql);
                Migration<T::CURRENT_VERSION, T>::forward(sql, version, _type);

                // register rollback migration
                _rollbackMigrations.emplace_back([type = _type](soci::session& sql) {
                    uint32_t const version = getDatabaseMigrationVersion<T>(sql);

                    soci::transaction tr(sql);
                    Migration<T::CURRENT_VERSION, T>::backward(sql, version, type);
                    tr.commit();
                });

                tr.commit();
            }

            /// Rollback all migrations for a given migration system.
            void rollbackMigrations() {
                soci::session sql(getPool());

                // rollback coins first
                for (auto& rollbackCoinMigration : _rollbackMigrations) {
                    rollbackCoinMigration(sql);
                }

                _rollbackMigrations.clear();

                uint32_t const version = getDatabaseMigrationVersion<CoreMigration>(sql);

                soci::transaction tr(sql);
                Migration<CoreMigration::CURRENT_VERSION, CoreMigration>::backward(sql, version, _type);
                tr.commit();
            }

            void performChangePassword(const std::string &oldPassword,
                                       const std::string &newPassword);

        private:
            std::shared_ptr<DatabaseBackend> _backend;
            soci::connection_pool _pool;
            std::ostream* _logger;
            LoggerStreamBuffer _buffer;
            api::DatabaseBackendType _type;

            // list of rollback migrations to play when dropping the database; this is a necessary
            // object to ensure we drop all “registered coin” before trying to remove everything from
            // the database
            std::vector<std::function<void (soci::session& sql)>> _rollbackMigrations;
        };
        
        /// Run migration forward for the ledger-core library.
        ///
        /// This is a special case because we don’t want to register ourselves for rollback,
        /// since we will call that function explicitly.
        template <>
        void DatabaseSessionPool::forwardMigration<CoreMigration>();
    }
}
