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

namespace ledger {
    namespace core {

        const int DatabaseSessionPool::POOL_SIZE = 24;

        FuturePtr<DatabaseSessionPool>
        DatabaseSessionPool::getSessionPool(const std::shared_ptr<api::ExecutionContext> &context,
                                            const std::shared_ptr<DatabaseBackend>& backend,
                                            const std::shared_ptr<api::PathResolver>& resolver,
                                            const std::string& dbName) {
            return FuturePtr<DatabaseSessionPool>::async(context, [backend, resolver, dbName] () {
                auto pool = std::shared_ptr<DatabaseSessionPool>(new DatabaseSessionPool(
                    backend, resolver, dbName, POOL_SIZE
                ));

                return pool;
            });
        }

        soci::connection_pool &DatabaseSessionPool::getPool() {
            return _pool;
        }

        void DatabaseSessionPool::performDatabaseMigration() {
            soci::session sql(getPool());
            int version = -1;
            try {
                sql << "SELECT version FROM __database_meta__ WHERE id == 0", soci::into(version);
            } catch (...) {
                // Ignore
            }
            soci::transaction tr(sql);
            migrate<CURRENT_DATABASE_SCHEME_VERSION>(sql, version);
            tr.commit();
        }

        DatabaseSessionPool::DatabaseSessionPool(const std::shared_ptr<DatabaseBackend> &backend,
                                                 const std::shared_ptr<api::PathResolver> &resolver,
                                                 const std::string &dbName, int poolSize) : _pool((size_t) poolSize) {
            for (size_t i = 0; i < poolSize; i++) {
                auto& session = getPool().at(i);
                backend->init(resolver, dbName, session);
            }
            // Migrate database
            performDatabaseMigration();
        }

    }
}