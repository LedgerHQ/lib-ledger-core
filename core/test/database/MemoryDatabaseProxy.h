/*
 *
 * MemoryDatabaseProxy.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/11/2018.
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

#ifndef LEDGER_CORE_MEMORYDATABASEPROXY_H
#define LEDGER_CORE_MEMORYDATABASEPROXY_H

#include <database/ProxyBackend.hpp>

class ConnectionPool;
class MemoryDatabaseProxy : public ledger::core::api::DatabaseEngine {
public:
    std::shared_ptr<ledger::core::api::DatabaseConnectionPool> connect(const std::string &connectUrl) override;

    int32_t getPoolSize() override;

    void changePassword(const std::string & oldPassword, const std::string & newPassword) override;

private:
    std::shared_ptr<ConnectionPool> _pool;
};


#endif //LEDGER_CORE_MEMORYDATABASEPROXY_H
