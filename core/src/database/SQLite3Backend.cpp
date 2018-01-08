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
#include <soci-sqlite3.h>
#include <utils/Exception.hpp>

using namespace soci;

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setUsername(const std::string &username) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setPassword(const std::string &pwd) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setHost(const std::string &host) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setHostAddr(const std::string &hostAddr) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setPort(const std::string &port) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setOptions(const std::string &opts) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setSslMode(const std::string &mode) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setKerberosName(const std::string &name) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setService(const std::string &service) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

void ledger::core::SQLite3Backend::init(const std::shared_ptr<ledger::core::api::PathResolver> &resolver,
                                        const std::string &dbName, soci::session &session) {
    session.open(*soci::factory_sqlite3(), resolver->resolveDatabasePath(dbName));
    session << "PRAGMA foreign_keys = ON";
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setConnectionPoolSize(int32_t size) {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getUsername() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getPassword() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getHost() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getHostAddr() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getPort() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getOptions() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getSslMode() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getKerberosName() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

std::string ledger::core::SQLite3Backend::getService() {
    throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "This operation is not supported by this backend");
}

int32_t ledger::core::SQLite3Backend::getConnectionPoolSize() {
    return 2;
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::enableQueryLogging(bool enable) {
    _logging = enable;
    return shared_from_this();
}

bool ledger::core::SQLite3Backend::isLoggingEnabled() {
    return _logging;
}

ledger::core::SQLite3Backend::SQLite3Backend() {
    _logging = false;
}
