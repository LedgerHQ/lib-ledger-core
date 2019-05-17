/*
 *
 * TezosLikeAccountDatabaseHelper
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#include "TezosLikeAccountDatabaseHelper.h"
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <crypto/SHA256.hpp>
#include <fmt/format.h>

using namespace soci;

namespace ledger {
    namespace core {
        void TezosLikeAccountDatabaseHelper::createAccount(soci::session &sql,
                                                           const std::string walletUid,
                                                           int32_t index,
                                                           const std::string &address) {
            auto uid = AccountDatabaseHelper::createAccountUid(walletUid, index);
            sql << "INSERT INTO tezos_accounts VALUES(:uid, :wallet_uid, :idx, :address)",use(uid), use(walletUid), use(index), use(address);
        }

        bool TezosLikeAccountDatabaseHelper::queryAccount(soci::session &sql,
                                                          const std::string &accountUid,
                                                          TezosLikeAccountDatabaseEntry &entry) {
            rowset<row> rows = (sql.prepare << "SELECT xtz.idx, xtz.address, "
                    "orig.uid, orig.address, orig.spendable, orig.delegatable, orig.public_key "
                    "FROM tezos_accounts AS xtz "
                    "LEFT JOIN tezos_originated_accounts AS orig ON xtz.uid = orig.tezos_account_uid "
                    "WHERE xtz.uid = :uid", use(accountUid));
            for (auto& row : rows) {
                if (entry.address.empty()) {
                    entry.index = row.get<int32_t>(0);
                    entry.address = row.get<std::string>(1);
                }
                // Get related originated accounts
                if (row.get_indicator(2) != i_null) {
                    TezosLikeOriginatedAccountDatabaseEntry originatedEntry;
                    originatedEntry.uid = row.get<std::string>(2);
                    originatedEntry.address = row.get<std::string>(3);
                    originatedEntry.spendable = row.get<bool>(4);
                    originatedEntry.delegatable = row.get<bool>(5);
                    if (row.get_indicator(6) != i_null) {
                        originatedEntry.publicKey = row.get<std::string>(6);
                    }
                    entry.originatedAccounts.emplace_back(originatedEntry);
                }
            }
            return !entry.address.empty();
        }

        std::string TezosLikeAccountDatabaseHelper::createOriginatedAccountUid(const std::string &xtzAccountUid, const std::string &originatedAddress) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}", xtzAccountUid, originatedAddress));
        }

        void TezosLikeAccountDatabaseHelper::updatePubKeyField(soci::session &sql, const std::string &accountUid, const std::string &pubKey) {
            sql << "UPDATE tezos_originated_accounts SET public_key = :public_key WHERE uid = :uid", use(pubKey), use(accountUid);
        }

    }
}