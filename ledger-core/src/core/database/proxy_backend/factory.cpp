/*
 *
 * factory.cpp
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
#include <connection-parameters.h>

using namespace soci;
using namespace ledger::core;

proxy_session_backend* proxy_backend_factory::make_session(const soci::connection_parameters &parameters) const {
    auto pool = get_pool(parameters);
    return new proxy_session_backend(pool->getConnection());
}

proxy_backend_factory::~proxy_backend_factory() {}

const std::shared_ptr<api::DatabaseEngine>& proxy_backend_factory::getEngine() const {
    return _engine;
}

std::shared_ptr<ledger::core::api::DatabaseConnectionPool>
proxy_backend_factory::get_pool(connection_parameters const &parameters) const {
    return _engine->connect(parameters.get_connect_string());
}

SOCI_PROXY_DECL backend_factory const* soci::factory_proxy(const std::shared_ptr<api::DatabaseEngine>& engine) {
    return new proxy_backend_factory(engine);
}