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
#ifndef LEDGER_CORE_WALLETPOOLDATABASE_HPP
#define LEDGER_CORE_WALLETPOOLDATABASE_HPP

#include "WalletPoolDatabaseWriter.hpp"
#include "WalletPoolDatabaseReader.hpp"
#include "../../database/DatabaseBackend.hpp"
#include "../../api/PathResolver.hpp"
#include <string>
#include <memory>

namespace ledger {
    namespace core {
        class WalletPoolDatabase {
        public:
            WalletPoolDatabase( const std::string& poolName,
                                const std::shared_ptr<api::PathResolver>& resolver,
                                const std::shared_ptr<DatabaseBackend>& backend
            );
            const WalletPoolDatabaseWriter& getWriter() const;
            const WalletPoolDatabaseReader& getReader() const;

        protected:
            WalletPoolDatabase(const std::shared_ptr<soci::session>& session);

        private:
            WalletPoolDatabaseWriter _writer;
            WalletPoolDatabaseReader _reader;
        };
    }
}


#endif //LEDGER_CORE_WALLETPOOLDATABASE_HPP
