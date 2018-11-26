/*
 *
 * session.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/11/2018.
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

#include "soci-proxy.h"

using namespace soci;
using namespace ledger::core;

proxy_session_backend::proxy_session_backend(const std::shared_ptr<ledger::core::api::DatabaseConnection> &connection) {
    _conn = connection;
}

proxy_session_backend::~proxy_session_backend() {

}

void proxy_session_backend::begin() {
    _conn->begin();
}

void proxy_session_backend::commit() {
    _conn->commit();
}

void proxy_session_backend::rollback() {
    _conn->rollback();
}

void proxy_session_backend::clean_up() {
    _conn->close();
}

proxy_statement_backend *proxy_session_backend::make_statement_backend() {
   return new proxy_statement_backend(*this);
}

proxy_rowid_backend *proxy_session_backend::make_rowid_backend() {
    // TODO: Create ROWID interface
    return nullptr;
}

proxy_blob_backend *proxy_session_backend::make_blob_backend() {
    // TODO: Create BLOB interface
    return nullptr;
}
