/*
 *
 * MysqlBackend.cpp
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

#include "MysqlBackend.h"
#define MYSQL_SUPPORT



namespace ledger {
    namespace core {

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setUsername(const std::string &username) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setPassword(const std::string &pwd) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setHost(const std::string &host) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setHostAddr(const std::string &hostAddr) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setPort(const std::string &port) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setOptions(const std::string &opts) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setSslMode(const std::string &mode) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setKerberosName(const std::string &name) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setService(const std::string &service) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::setConnectionPoolSize(int32_t size) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::shared_ptr<api::DatabaseBackend> MysqlBackend::enableQueryLogging(bool enable) {
            return std::shared_ptr<api::DatabaseBackend>();
        }

        std::string MysqlBackend::getUsername() {
            return std::string();
        }

        std::string MysqlBackend::getPassword() {
            return std::string();
        }

        std::string MysqlBackend::getHost() {
            return std::string();
        }

        std::string MysqlBackend::getHostAddr() {
            return std::string();
        }

        std::string MysqlBackend::getPort() {
            return std::string();
        }

        std::string MysqlBackend::getOptions() {
            return std::string();
        }

        std::string MysqlBackend::getSslMode() {
            return std::string();
        }

        std::string MysqlBackend::getKerberosName() {
            return std::string();
        }

        std::string MysqlBackend::getService() {
            return std::string();
        }

        int32_t MysqlBackend::getConnectionPoolSize() {
            return 0;
        }

        bool MysqlBackend::isLoggingEnabled() {
            return false;
        }

        MysqlBackend::MysqlBackend() {

        }

        void MysqlBackend::init(const std::shared_ptr<api::PathResolver> &resolver, const std::string &dbName,
                                soci::session &session) {

        }
    }
}