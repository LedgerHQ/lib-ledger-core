/*
 *
 * StellarLikeAccountDatabaseHelper.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 11/07/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#ifndef LEDGER_CORE_STELLARLIKEACCOUNTDATABASEHELPER_HPP
#define LEDGER_CORE_STELLARLIKEACCOUNTDATABASEHELPER_HPP

#include <wallet/stellar/stellar.hpp>
#include <soci.h>

namespace ledger {
    namespace core {
        class StellarLikeAccountDatabaseHelper {
        public:
            StellarLikeAccountDatabaseHelper() = delete;

            static bool getAccount(soci::session& sql, const std::string& accountUid, stellar::Account& out);
            static void getAccountBalances(soci::session& sql, const std::string& accountUid, stellar::Account& out);
            static void putAccount(soci::session& sql, const std::string& walletUid, int32_t accountIndex, const stellar::Account& in);
            static void createAccount(soci::session& sql, const std::string& walletUid, int32_t accountIndex, const stellar::Account& in);

            static void getAccountSigners(soci::session& sql, const std::string& accountUid, std::vector<stellar::AccountSigner>& signers);
            static void putAccountSigner(soci::session& sql, const std::string& accountUid, const stellar::AccountSigner& signer);
            static bool putAccountBalance(soci::session& sql, const std::string& accountUid, const stellar::Balance& balance);
            static std::string createAccountBalanceUid(const std::string& accountUid, const std::string& assetUid);

        };
    }
}


#endif //LEDGER_CORE_STELLARLIKEACCOUNTDATABASEHELPER_HPP
