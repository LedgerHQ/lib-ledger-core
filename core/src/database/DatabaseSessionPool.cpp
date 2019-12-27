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

#include "DatabaseSessionPool.hpp"
#include "migrations.hpp"
#include "PostgreSQLBackend.h"

namespace ledger {
    namespace core {
        DatabaseSessionPool::DatabaseSessionPool(
            const std::shared_ptr<DatabaseBackend> &backend,
            const std::shared_ptr<api::PathResolver> &resolver,
            const std::shared_ptr<spdlog::logger>& logger,
            const std::string &dbName,
            const std::string &password) :
            _pool((size_t) backend->getConnectionPoolSize()), _backend(backend), _buffer("SQL", logger) {
            if (logger != nullptr && backend->isLoggingEnabled()) {
                _logger = new std::ostream(&_buffer);
            } else {
                _logger = nullptr;
            }

            auto poolSize = _backend->getConnectionPoolSize();
            for (size_t i = 0; i < poolSize; i++) {
                auto& session = getPool().at(i);
                _backend->init(resolver, dbName, password, session);
                if (_logger != nullptr)
                    session.set_log_stream(_logger);
            }

            _type = std::dynamic_pointer_cast<PostgreSQLBackend>(backend) != nullptr ?
                    api::DatabaseBackendType::POSTGRESQL : api::DatabaseBackendType::SQLITE3;
            // Migrate database
            performDatabaseMigration();
        }

        DatabaseSessionPool::~DatabaseSessionPool() {
            delete _logger;
        }

        FuturePtr<DatabaseSessionPool>
        DatabaseSessionPool::getSessionPool(const std::shared_ptr<api::ExecutionContext> &context,
                                            const std::shared_ptr<DatabaseBackend> &backend,
                                            const std::shared_ptr<api::PathResolver> &resolver,
                                            const std::shared_ptr<spdlog::logger> &logger,
                                            const std::string &dbName,
                                            const std::string &password) {
            return FuturePtr<DatabaseSessionPool>::async(context, [backend, resolver, dbName, logger, password] () {
                auto pool = std::shared_ptr<DatabaseSessionPool>(new DatabaseSessionPool(
                    backend, resolver, logger, dbName, password
                ));

                return pool;
            });
        }

        soci::connection_pool &DatabaseSessionPool::getPool() {
            return _pool;
        }

        void DatabaseSessionPool::performDatabaseMigration() {
            soci::session sql(getPool());
            int version = getDatabaseMigrationVersion(sql);

            soci::transaction tr(sql);
            migrate<CURRENT_DATABASE_SCHEME_VERSION>(sql, version, _type);
            tr.commit();
        }

        void DatabaseSessionPool::performDatabaseRollback() {
            soci::session sql(getPool());
            int version = getDatabaseMigrationVersion(sql);

            soci::transaction tr(sql);
            rollback<CURRENT_DATABASE_SCHEME_VERSION>(sql, version, _type);
            tr.commit();
        }

        void DatabaseSessionPool::performChangePassword(const std::string &oldPassword,
                                                        const std::string &newPassword) {
            auto poolSize = _backend->getConnectionPoolSize();
            for (size_t i = 0; i < poolSize; i++) {
                auto& session = getPool().at(i);
                _backend->changePassword(oldPassword, newPassword, session);
            }
        }
    }
}
