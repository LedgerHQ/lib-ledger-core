/*
 *
 * WalletPoolDatabase
 * ledger-core
 *
 * Created by Pierre Pollastri on 22/12/2016.
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
#include "WalletPoolDatabase.hpp"
#include "../../ledger-core.h"

using namespace soci;

const int ledger::core::WalletPoolDatabase::DATABASE_VERSION = WALLET_POOL_DATABASE_VERSION;

ledger::core::WalletPoolDatabase::WalletPoolDatabase(const std::string &poolName,
                                                     const std::shared_ptr<ledger::core::api::PathResolver> &resolver,
                                                     const std::shared_ptr<ledger::core::DatabaseBackend> &backend) :
 WalletPoolDatabase(backend->makeSession(resolver, poolName + ".db")) {

}

ledger::core::WalletPoolDatabase::WalletPoolDatabase(const std::shared_ptr<soci::session> &session) :
    _writer(session), _reader(session)
{
    try {
        int structureVersion, libVersion;
        *session << "SELECT structure_version, lib_version from configuration", into(structureVersion), into(
                libVersion);
    } catch (soci::soci_error& e) {
        // Table probably doesn't exist
        *session << "CREATE TABLE configuration(structure_version INTEGER NOT NULL, lib_version INTEGER NOT NULL)";
        *session << "INSERT INTO configuration(structure_version, lib_version) VALUES(:s, :l)", use(DATABASE_VERSION), use(LIB_VERSION);
    }
}

const ledger::core::WalletPoolDatabaseWriter &ledger::core::WalletPoolDatabase::getWriter() const {
    return _writer;
}

const ledger::core::WalletPoolDatabaseReader &ledger::core::WalletPoolDatabase::getReader() const {
    return _reader;
}


