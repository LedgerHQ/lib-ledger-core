/*
 *
 * ProxyBackend.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/11/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "ProxyBackend.hpp"
#include <database/proxy_backend/soci-proxy.h>

namespace ledger {
    namespace core {

        ProxyBackend::ProxyBackend(const std::shared_ptr<ledger::core::api::DatabaseEngine> &engine) :
            DatabaseBackend(), _engine(engine), _factory(soci::factory_proxy(engine)) {
        }

        int32_t ProxyBackend::getConnectionPoolSize() {
            return _engine->getPoolSize();
        }

        int32_t ProxyBackend::getReadonlyConnectionPoolSize() {
            return _engine->getReadonlyPoolSize();
        }

        void ProxyBackend::init(const std::shared_ptr<api::PathResolver> &resolver,
                                const std::string &dbName,
                                const std::string &password,
                                soci::session &session) {
            _dbName = dbName;
            session.open(*_factory, dbName);
            if (!password.empty()) {
                setPassword(password, session);
            }
        }

        void ProxyBackend::setPassword(const std::string &password,
                                       soci::session &session) {
            if (_dbName.empty()) {
                throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "Database should be initiated before setting password.");
            }
            auto parameters = fmt::format("dbname=\"{}\" ", _dbName) + fmt::format("key=\"{}\" ", password);
            session.close();
            session.open(*_factory, parameters);
        }

        void ProxyBackend::changePassword(const std::string & oldPassword,
                                          const std::string & newPassword,
                                          soci::session &session) {
            if (_dbName.empty()) {
                throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "Database should be initiated before changing password.");
            }

            std::string db_params;
            if (!newPassword.empty()) {
                // change password if a new password is provided
                db_params = fmt::format("dbname=\"{}\" ", _dbName) + fmt::format("key=\"{}\" ", oldPassword) + fmt::format("new_key=\"{}\" ", newPassword);
            } else {
                // otherwise, decrypt the database back to plain text by providing an empty new key
                db_params = fmt::format("dbname=\"{}\" ", _dbName) + fmt::format("key=\"{}\" ", oldPassword) + "disable_encryption=\"true\" ";
            }

            session.close();
            session.open(*_factory, db_params);
        }

        ProxyBackend::~ProxyBackend() {
            delete _factory;
        }
    }
}
