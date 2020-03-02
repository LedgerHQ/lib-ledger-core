/*
 *
 * PostgreSQLBackend
 *
 * Created by El Khalil Bellakrid on 06/12/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include <soci-postgresql.h>

#ifdef SSL_SUPPORT
    #include <thread>
    #include <openssl/ssl.h>
#endif

#include <core/api/ConfigurationDefaults.hpp>
#include <core/database/PostgreSQLBackend.hpp>
#include <core/utils/Exception.hpp>

using namespace soci;

namespace ledger {
    namespace core {
        PostgreSQLBackend::PostgreSQLBackend() : DatabaseBackend(),
                                                 _connectionPoolSize(api::ConfigurationDefaults::DEFAULT_PG_CONNECTION_POOL_SIZE)
        {
            initSSLLibraries();
        }

        PostgreSQLBackend::PostgreSQLBackend(int32_t connectionPoolSize) : DatabaseBackend(),
                                                                           _connectionPoolSize(connectionPoolSize)
        {}

        int32_t PostgreSQLBackend::getConnectionPoolSize() {
            return _connectionPoolSize;
        }

        void PostgreSQLBackend::init(const std::shared_ptr<ledger::core::api::PathResolver> &resolver,
                                  const std::string &dbName,
                                  const std::string &password,
                                  soci::session &session) {
            _dbName = dbName;
            setPassword(password, session);
        }

        void PostgreSQLBackend::setPassword(const std::string &password,
                                         soci::session &session) {
            if (_dbName.empty()) {
                throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "Database should be initiated before setting password.");
            }
            session.close();
            session.open(*soci::factory_postgresql(), _dbName);
        }

        void PostgreSQLBackend::changePassword(const std::string & oldPassword,
                                            const std::string & newPassword,
                                            soci::session &session) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Change of password is not handled with PostgreSQL backend.");
        }

        std::once_flag PostgreSQLBackend::sslFlag;
        void PostgreSQLBackend::initSSLLibraries() {
#ifdef SSL_SUPPORT
            std::call_once(PostgreSQLBackend::sslFlag, [] () {
            #if OPENSSL_VERSION_NUMBER < 0x10100000L
                SSL_library_init();
            #else
                OPENSSL_init_ssl(0, NULL);
            #endif
            });
#endif
        }
    }
}
