/*
 *
 * OperationDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 31/05/2017.
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
#ifndef LEDGER_CORE_OPERATIONDATABASEHELPER_H
#define LEDGER_CORE_OPERATIONDATABASEHELPER_H

#include <api/OperationType.hpp>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/Operation.h>
#include <soci.h>
#include <string>

namespace ledger {
    namespace core {
        class OperationDatabaseHelper {
        public:
            static bool putOperation(soci::session& sql,
                                     const std::shared_ptr<AbstractAccount> &account,
                                     const Operation& operation);
            static std::string createUid(const std::string& accountUid,
                                         const std::string& txId,
                                         const api::OperationType type);
            static void queryOperations(soci::session& sql, int32_t from, int32_t to,
                                        bool complete, bool excludeDropped, std::vector<Operation>& out);

            static std::size_t queryOperations(soci::session &sql,
                                               const std::string &accountUid,
                                               std::vector<Operation>& out,
                                               std::function<bool (const std::string& address)> filter);
        private:
            static void updateCurrencyOperation(soci::session& sql,
                                                const std::shared_ptr<AbstractAccount> &account,
                                                const Operation& operation,
                                                bool insert);
        };
    }
}


#endif //LEDGER_CORE_OPERATIONDATABASEHELPER_H
