/*
 *
 * EthereumLikeAccountDatabaseHelper
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#include "EthereumLikeAccountDatabaseHelper.h"
#include <wallet/common/database/AccountDatabaseHelper.h>

using namespace soci;

namespace ledger {
    namespace core {
        void EthereumLikeAccountDatabaseHelper::createAccount(soci::session &sql,
                                                         const std::string walletUid, int32_t index,
                                                         const std::string &xpub) {
            auto uid = AccountDatabaseHelper::createAccountUid(walletUid, index);
            sql << "INSERT INTO ethereum_accounts VALUES(:uid, :wallet, :idx, :xpub)",use(uid), use(walletUid), use(index), use(xpub);
        }

        bool EthereumLikeAccountDatabaseHelper::queryAccount(soci::session &sql,
                                                             const std::string &accountUid,
                                                             EthereumLikeAccountDatabaseEntry &entry) {
            rowset<row> rows = (sql.prepare << "SELECT idx, xpub FROM ethereum_accounts WHERE uid = :uid", use(accountUid));
            for (auto& row : rows) {
                entry.index = row.get<int32_t>(0);
                entry.xpub = row.get<std::string>(1);
                return true;
            }
            return false;
        }

    }
}