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

#include "PostgreSQLBackend.h"
#include "migrations.hpp"

#include <api/Span.hpp>
#include <memory>
#include <utility>

namespace ledger {
    namespace core {
        namespace impl {
            class SessionTracer : public soci::session_tracer {
              public:
                ~SessionTracer() override                       = default;
                SessionTracer(const SessionTracer &)            = delete;
                SessionTracer(SessionTracer &&)                 = delete;
                SessionTracer &operator=(SessionTracer &&)      = delete;
                SessionTracer &operator=(const SessionTracer &) = delete;

                explicit SessionTracer(std::shared_ptr<api::CoreTracer> tracer) : _tracer(std::move(tracer)) {
                }

                int startSpan(std::string &string) override {
                    const std::lock_guard<std::mutex> lock(mut);
                    auto span = _tracer->startSpan(string);
                    span->setTagStr("db.statement", string);
                    span->setTagStr("sql.query", string);
                    span->setTagStr("db.system", "postgresql");
                    span->setTagStr("span.type", "sql");
                    span->setTagStr("service", "postgresql");
                    int id = idx++;
                    _traces.insert(std::make_pair(id, span));
                    return id;
                }

                void finishSpan(int traceId) override {
                    const std::lock_guard<std::mutex> lock(mut);
                    if (_traces.count(traceId) == 0) {
                        throw std::runtime_error("Unknown session trace id");
                    }
                    _traces.at(traceId)->close();
                    _traces.erase(traceId);
                }

              private:
                std::atomic<int> idx{};
                std::map<int, std::shared_ptr<api::Span>> _traces;
                std::shared_ptr<api::CoreTracer> _tracer;
                std::mutex mut{};
            };
        } // namespace impl

        DatabaseSessionPool::DatabaseSessionPool(
            const std::shared_ptr<DatabaseBackend> &backend,
            const std::shared_ptr<api::PathResolver> &resolver,
            const std::shared_ptr<api::CoreTracer> &tracer,
            const std::shared_ptr<spdlog::logger> &logger,
            const std::string &dbName,
            const std::string &password) : _pool(static_cast<size_t>(backend->getConnectionPoolSize()), std::make_shared<impl::SessionTracer>((tracer))),
                                           _readonlyPool(static_cast<size_t>(backend->getReadonlyConnectionPoolSize()), std::make_shared<impl::SessionTracer>((tracer))),
                                           _backend(backend),
                                           _buffer("SQL", logger) {
            std::cout << "DatabaseSessionPool::DatabaseSessionPool" << std::endl;
            if (logger != nullptr && backend->isLoggingEnabled()) {
                _logger = new std::ostream(&_buffer);
            } else {
                _logger = nullptr;
            }

            auto poolSize = _backend->getConnectionPoolSize();
            for (size_t i = 0; i < poolSize; i++) {
                auto &session = getPool().at(i);
                _backend->init(resolver, dbName, password, session);
                if (_logger != nullptr) {
                    session.set_log_stream(_logger);
                }
            }

            _type = api::DatabaseBackendType::POSTGRESQL;
            if (std::dynamic_pointer_cast<PostgreSQLBackend>(backend) == nullptr) {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Libcore supports only PostgreSQL backend.");
            }

            // Migrate database
            performDatabaseMigration();
            auto readonlyPoolSize = _backend->getReadonlyConnectionPoolSize();
            for (size_t i = 0; i < readonlyPoolSize; i++) {
                auto &session = getReadonlyPool().at(i);
                _backend->init(resolver, dbName, password, session);
                if (_logger != nullptr) {
                    session.set_log_stream(_logger);
                }
            }
        }

        DatabaseSessionPool::~DatabaseSessionPool() {
            delete _logger;
        }

        FuturePtr<DatabaseSessionPool>
        DatabaseSessionPool::getSessionPool(const std::shared_ptr<api::ExecutionContext> &context,
                                            const std::shared_ptr<DatabaseBackend> &backend,
                                            const std::shared_ptr<api::PathResolver> &resolver,
                                            const std::shared_ptr<api::CoreTracer> &tracer,
                                            const std::shared_ptr<spdlog::logger> &logger,
                                            const std::string &dbName,
                                            const std::string &password) {
            return FuturePtr<DatabaseSessionPool>::async(context, [backend, resolver, tracer, dbName, logger, password]() {
                auto pool = std::make_shared<DatabaseSessionPool>(
                    backend, resolver, tracer, logger, dbName, password);

                return pool;
            });
        }

        soci::connection_pool &DatabaseSessionPool::getPool() {
            return _pool;
        }

        soci::connection_pool &DatabaseSessionPool::getReadonlyPool() {
            return _readonlyPool;
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
                auto &session = getPool().at(i);
                _backend->changePassword(oldPassword, newPassword, session);
            }
        }

        bool DatabaseSessionPool::isPostgres() const {
            return std::dynamic_pointer_cast<PostgreSQLBackend>(_backend) != nullptr;
        }
    } // namespace core
} // namespace ledger
