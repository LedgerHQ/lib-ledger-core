/*
 *
 * MysqlBackend.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 23/04/2018.
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

#ifndef LEDGER_CORE_MYSQLBACKEND_H
#define LEDGER_CORE_MYSQLBACKEND_H

#include "DatabaseBackend.hpp"

namespace ledger {
    namespace core {
        class MysqlBackend : public DatabaseBackend, public std::enable_shared_from_this<MysqlBackend> {
        public:
            MysqlBackend();
            std::shared_ptr<api::DatabaseBackend> setUsername(const std::string &username) override;
            std::shared_ptr<api::DatabaseBackend> setPassword(const std::string &pwd) override;
            std::shared_ptr<api::DatabaseBackend> setHost(const std::string &host) override;
            std::shared_ptr<api::DatabaseBackend> setHostAddr(const std::string &hostAddr) override;
            std::shared_ptr<api::DatabaseBackend> setPort(const std::string &port) override;
            std::shared_ptr<api::DatabaseBackend> setOptions(const std::string &opts) override;
            std::shared_ptr<api::DatabaseBackend> setSslMode(const std::string &mode) override;
            std::shared_ptr<api::DatabaseBackend> setKerberosName(const std::string &name) override;
            std::shared_ptr<api::DatabaseBackend> setService(const std::string &service) override;
            std::shared_ptr<api::DatabaseBackend> setConnectionPoolSize(int32_t size) override;
            std::shared_ptr<api::DatabaseBackend> enableQueryLogging(bool enable) override;
            std::string getUsername() override;
            std::string getPassword() override;
            std::string getHost() override;
            std::string getHostAddr() override;
            std::string getPort() override;
            std::string getOptions() override;
            std::string getSslMode() override;
            std::string getKerberosName() override;
            std::string getService() override;
            int32_t getConnectionPoolSize() override;
            bool isLoggingEnabled() override;

            void init(const std::shared_ptr<api::PathResolver> &resolver, const std::string &dbName,
                      soci::session &session) override;

        private:
            bool _logging;
            int32_t _poolSize;

        };
    }
}

#endif //LEDGER_CORE_MYSQLBACKEND_H
