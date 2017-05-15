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

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setUsername(const std::string &username) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setPassword(const std::string &pwd) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setHost(const std::string &host) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setHostAddr(const std::string &hostAddr) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setPort(const std::string &port) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setOptions(const std::string &opts) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend> ledger::core::SQLite3Backend::setSslMode(const std::string &mode) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setKerberosName(const std::string &name) {
    return nullptr;
}

std::shared_ptr<ledger::core::api::DatabaseBackend>
ledger::core::SQLite3Backend::setService(const std::string &service) {
    return nullptr;
}

#include <soci-sqlite3.h>

using namespace soci;

void ledger::core::SQLite3Backend::init(const std::shared_ptr<ledger::core::api::PathResolver> &resolver,
                                        const std::string &dbName, soci::session &session) {
    session.open(*soci::factory_sqlite3(), resolver->resolveDatabasePath(dbName));
    session << "PRAGMA foreign_keys = ON";
}