/*
 *
 * AccountDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/05/2017.
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
#ifndef LEDGER_CORE_ACCOUNTDATABASEHELPER_H
#define LEDGER_CORE_ACCOUNTDATABASEHELPER_H

#include <string>
#include <soci.h>
#include <list>
#include <api/Block.hpp>
#include <utils/Option.hpp>
namespace ledger {
    namespace core {

        class AccountDatabaseHelper {
        public:
            static bool accountExists(soci::session& sql, const std::string& walletUid, int32_t index);
            static int32_t getAccountsCount(soci::session& sql, const std::string& walletUid);
            static void createAccount(soci::session& sql, const std::string& walletUid, int32_t index);
            static void removeAccount(soci::session& sql, const std::string& walletUid, int32_t index);
            static std::string createAccountUid(const std::string& walletUid, int32_t accountIndex);
            static std::string createERC20AccountUid(const std::string &ethAccountUid, const std::string &contractAddress);
            static int32_t computeNextAccountIndex(soci::session& sql, const std::string& walletUid);
            static std::list<int32_t>& getAccountsIndexes(soci::session& sql, const std::string& walletUid, int32_t from, int32_t count, std::list<int32_t>& out);
            static Option<api::Block> getLastBlockWithOperations(soci::session &sql, const std::string &accountUid);
        };
    }
}


#endif //LEDGER_CORE_ACCOUNTDATABASEHELPER_H
