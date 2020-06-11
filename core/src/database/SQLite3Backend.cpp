/*
 *
 * SQLite3Backend
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
#include "SQLite3Backend.hpp"
#include <utils/Exception.hpp>

using namespace soci;

namespace ledger {
    namespace core {
        SQLite3Backend::SQLite3Backend() : DatabaseBackend() {
        }

        int32_t SQLite3Backend::getConnectionPoolSize() {
            return 1;
        }

        void SQLite3Backend::init(const std::shared_ptr<ledger::core::api::PathResolver> &resolver,
                                  const std::string &dbName,
                                  const std::string &password,
                                  soci::session &session) {
            _dbResolvedPath = resolver->resolveDatabasePath(dbName);
            setPassword(password, session);
            session << "PRAGMA foreign_keys = ON";
            // All settings below can only be active during tests, the goal is to lighten the
            // I/O load on the machine because it might end up in corrupting tests
            //
            // DELETE or WAL here will throw a lot of I/O errors during parallel tests
            session << "PRAGMA journal_mode = MEMORY";
            session << "PRAGMA synchronous = NORMAL";
            session << "PRAGMA temp_store = MEMORY";
        }

        void SQLite3Backend::setPassword(const std::string &password,
                                         soci::session &session) {
            if (_dbResolvedPath.empty()) {
                throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "Database should be initiated before setting password.");
            }
            auto parameters = fmt::format("dbname=\"{}\" ", _dbResolvedPath) + fmt::format("key=\"{}\" ", password);
            session.close();
            session.open(*soci::factory_sqlite3(), parameters);
        }

        void SQLite3Backend::changePassword(const std::string & oldPassword,
                                            const std::string & newPassword,
                                            soci::session &session) {
            if (_dbResolvedPath.empty()) {
                throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "Database should be initiated before changing password.");
            }

            auto db_params = fmt::format("dbname=\"{}\" ", _dbResolvedPath) + fmt::format("key=\"{}\" ", oldPassword);
            if (!newPassword.empty()) {
                // change password if a new password is provided
                db_params += fmt::format("new_key=\"{}\" ", newPassword);
            } else {
                // otherwise, decrypt the database back to plain text by providing an empty new key
                db_params += "disable_encryption=\"true\" ";
            }

            session.close();
            session.open(*soci::factory_sqlite3(), db_params);

            // Temporary workaround
            // TODO: need investigation
            db_params = fmt::format("dbname=\"{}\" ", _dbResolvedPath) + fmt::format("key=\"{}\" ", newPassword);
            session.close();
            session.open(*soci::factory_sqlite3(), db_params);
        }
    }
}
