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

#include <soci.h>
#include <sqlite3/soci-sqlite3.h>
#include <iostream>

using namespace soci;

std::shared_ptr<soci::session> ledger::core::SQLite3Backend::makeSession(const std::string &dbName) {
    backend_factory const &backEnd = *soci::factory_sqlite3();
    soci::session sql(backEnd, "toto_is_back.db");
    try { sql << "drop table test1"; }
    catch (soci_error const &) {} // ignore if error

    sql <<
        "create table test1 ("
                "    id integer,"
                "    name varchar(100)"
                ")";

    sql << "insert into test1(id, name) values(7, \'John\')";

    rowid rid(sql);
    sql << "select oid from test1 where id = 7", into(rid);

    int id;
    std::string name;

    sql << "select id, name from test1 where oid = :rid",
            into(id), into(name), use(rid);

    std::cout << "Got " << id << " " << name << std::endl;
    sql << "drop table test1";
    return nullptr;
}
