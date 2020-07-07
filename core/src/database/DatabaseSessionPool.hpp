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

#ifndef LEDGER_CORE_DATABASESESSIONPOOL_HPP
#define LEDGER_CORE_DATABASESESSIONPOOL_HPP

#include <soci.h>
#include <api/ExecutionContext.hpp>
#include <async/Future.hpp>
#include <database/DatabaseBackend.hpp>
#include <debug/LoggerStreamBuffer.h>
#include <api/DatabaseBackendType.hpp>

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

            static const int CURRENT_DATABASE_SCHEME_VERSION = 22;

            void performDatabaseMigration();
            void performDatabaseRollback();
            void performChangePassword(const std::string &oldPassword,
                                       const std::string &newPassword);

        private:
            std::shared_ptr<DatabaseBackend> _backend;
            soci::connection_pool _pool;
            std::ostream* _logger;
            LoggerStreamBuffer _buffer;
            api::DatabaseBackendType _type;
        };
    }
}

#endif //LEDGER_CORE_DATABASESESSIONPOOL_HPP
