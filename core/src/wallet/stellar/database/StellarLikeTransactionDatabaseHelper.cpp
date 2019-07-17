/*
 *
 * StellarLikeTransactionDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/07/2019.
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

#include "StellarLikeTransactionDatabaseHelper.hpp"
#include <crypto/SHA256.hpp>
#include <fmt/format.h>

using namespace soci;

namespace ledger {
    namespace core {

        bool StellarLikeTransactionDatabaseHelper::putTransaction(soci::session &sql, const api::Currency &currency,
                                                                  const stellar::Transaction &tx) {
            auto uid = createTransactionUid(currency, tx.hash);
            if (!transactionExists(sql, uid)) {
                auto ledger = BigInt(tx.ledger).toString();
                auto fee = tx.feePaid.toString();
                sql << "INSERT INTO stellar_transactions VALUES(:uid, :hash, :account, :fee, :success, :ledger)",
                        use(uid), use(tx.hash), use(tx.sourceAccount), use(fee), use(tx.successful ? 1 : 0),
                        use(ledger), use(tx.memoType), use(tx.memo);
                return true;
            }
            return false;
        }

        std::string StellarLikeTransactionDatabaseHelper::createTransactionUid(const api::Currency &currency,
                const std::string &transactionHash) {
            return SHA256::stringToHexHash(fmt::format("{}::{}", currency.name, transactionHash));
        }

        bool StellarLikeTransactionDatabaseHelper::transactionExists(soci::session &sql, const std::string &uid) {
            auto count = 0;
            sql << "SELECT COUNT(*) FROM stellar_transactions WHERE uid = :uid", use(uid), into(count);
            return count > 0;
        }

        std::string
        StellarLikeTransactionDatabaseHelper::putOperation(soci::session &sql, const std::string &accountUid,
                                                           const stellar::Operation &operation) {
            return std::string();
        }

        bool
        StellarLikeTransactionDatabaseHelper::getOperation(soci::session &sql, const std::string &accountOperationUid,
                                                           stellar::Operation &out) {

            return false;
        }

        bool StellarLikeTransactionDatabaseHelper::getTransaction(soci::session &sql, const std::string &hash,
                                                                  stellar::Transaction &out) {
            std::string fee;
            int successful;
            sql << "SELECT hash, source_account, fee, successful, ledger, memo_type, memo "
                   "FROM stellar_transactions "
                   "WHERE hash = :hash LIMIT 1", use(hash), into(out.hash), into(out.sourceAccount), into(fee),
                   into(successful), into(out.ledger), into(out.memoType), into(out.memo);
            out.successful = successful == 1;
            out.feePaid = BigInt::fromString(fee);
            return out.hash == hash;
        }
    }
}