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
#ifndef LEDGER_CORE_SQLITE3BACKEND_HPP
#define LEDGER_CORE_SQLITE3BACKEND_HPP

#include "DatabaseBackend.hpp"
#include <memory>

namespace ledger {
 namespace core {
     class SQLite3Backend : public DatabaseBackend {
     public:
         SQLite3Backend();
         int32_t getConnectionPoolSize() override;

         void init(const std::shared_ptr<api::PathResolver> &resolver,
                   const std::string &dbName,
                   const std::string &password,
                   soci::session &session) override;

     private:
        bool _logging;
     };
 }
}


#endif //LEDGER_CORE_SQLITE3BACKEND_HPP
