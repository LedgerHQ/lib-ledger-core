/*
 *
 * ProxyBackend.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/11/2018.
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

#pragma once

#include <core/api/DatabaseEngine.hpp>
#include <core/database/DatabaseBackend.hpp>

namespace ledger {
    namespace core {
        class ProxyBackend : public DatabaseBackend {
        public:
            explicit ProxyBackend(const std::shared_ptr<api::DatabaseEngine>& engine);
            ~ProxyBackend();

            int32_t getConnectionPoolSize() override;

            void init(
                const std::shared_ptr<api::PathResolver> &resolver,
                const std::string &dbName,
                const std::string &password,
                soci::session &session
            ) override;

            void setPassword(
                const std::string &password,
                soci::session &session
            ) override;

            void changePassword(
                const std::string &oldPassword,
                const std::string &newPassword,
                soci::session &session
            ) override;

        private:
            std::shared_ptr<api::DatabaseEngine> _engine;
            const soci::backend_factory* _factory;
            std::string _dbName;
        };
    }
}
