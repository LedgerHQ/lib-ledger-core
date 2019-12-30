/*
 *
 * DatabaseBackend
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/12/2016.
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
#include "DatabaseBackend.hpp"
#include "SQLite3Backend.hpp"
#ifdef PG_SUPPORT
#include "PostgreSQLBackend.h"
#endif
#include <api/DatabaseEngine.hpp>
#include <api/Error.hpp>
#include <utils/Exception.hpp>
#include "ProxyBackend.hpp"

namespace ledger {
    namespace core {

        std::shared_ptr<api::DatabaseBackend> api::DatabaseBackend::getSqlite3Backend() {
            return std::make_shared<SQLite3Backend>();
        }

        std::shared_ptr<api::DatabaseBackend> api::DatabaseBackend::getPostgreSQLBackend(int32_t connectionPoolSize) {
#ifdef PG_SUPPORT
            return std::make_shared<PostgreSQLBackend>(connectionPoolSize);
#else
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Libcore should be compiled with PG_SUPPORT flag.");
#endif
        }

        std::shared_ptr<api::DatabaseBackend> api::DatabaseBackend::createBackendFromEngine(
                const std::shared_ptr<ledger::core::api::DatabaseEngine> &engine) {
            return std::make_shared<ProxyBackend>(engine);
        }

        std::shared_ptr<api::DatabaseBackend> DatabaseBackend::enableQueryLogging(bool enable) {
            _enableLogging = enable;
            return shared_from_this();
        }

        bool DatabaseBackend::isLoggingEnabled() {
            return _enableLogging;
        }
    }
}