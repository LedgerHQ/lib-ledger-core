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
#include "StellarLikeAssetDatabaseHelper.hpp"
#include <database/soci-date.h>
#include <database/soci-option.h>

using namespace soci;

namespace ledger {
    namespace core {

        bool StellarLikeTransactionDatabaseHelper::putTransaction(soci::session &sql, const api::Currency &currency,
                                                                  const stellar::Transaction &tx) {
            auto uid = createTransactionUid(currency.name, tx.hash);
            if (!transactionExists(sql, uid)) {
                auto ledger = BigInt(tx.ledger).toString();
                auto fee = tx.feePaid.toString();
                sql << "INSERT INTO stellar_transactions VALUES(:uid, :hash, :account, :fee, :success, :ledger,"
                       ":memo_typ, :memo)",
                        use(uid), use(tx.hash), use(tx.sourceAccount), use(fee), use(tx.successful ? 1 : 0),
                        use(ledger), use(tx.memoType), use(tx.memo);
                return true;
            }
            return false;
        }

        std::string StellarLikeTransactionDatabaseHelper::createTransactionUid(const std::string &currencyName,
                const std::string &transactionHash) {
            return SHA256::stringToHexHash(fmt::format("{}::{}", currencyName, transactionHash));
        }

        bool StellarLikeTransactionDatabaseHelper::transactionExists(soci::session &sql, const std::string &uid) {
            auto count = 0;
            sql << "SELECT COUNT(*) FROM stellar_transactions WHERE uid = :uid", use(uid), into(count);
            return count > 0;
        }

        std::string
        StellarLikeTransactionDatabaseHelper::putOperation(soci::session &sql, const std::string &accountUid,
                                                           const std::string& currencyName,
                                                           const stellar::Operation &op) {
            auto uid = createOperationUid(accountUid, op.id);
            auto txUid = createTransactionUid(currencyName, op.transactionHash);
            auto assetUid = StellarLikeAssetDatabaseHelper::createAssetUid(op.asset);
            auto sourceAssetUid = op.sourceAsset.map<std::string>([] (const stellar::Asset& asset) {
                return StellarLikeAssetDatabaseHelper::createAssetUid(asset);
            });
            auto amount = op.amount.toString();
            auto srcAmount = op.sourceAmount.map<std::string>([] (const BigInt& b) {
                return b.toString();
            });
            StellarLikeAssetDatabaseHelper::putAsset(sql, op.asset);
            if (op.sourceAsset)  {
                StellarLikeAssetDatabaseHelper::putAsset(sql, op.sourceAsset.getValue());
            }
            int type = (int)op.type;
            if (!operationExists(sql, uid)) {
                sql << "INSERT INTO stellar_operations VALUES("
                       ":uid, :tx_uid, :hash, :time, :asset_uid, :src_asset_uid, :amount, :src_amount, :from, :to, :type"
                       ")", use(uid), use(txUid), use(op.id), use(op.createdAt), use(assetUid), use(sourceAssetUid),
                       use(amount), use(srcAmount), use(op.from), use(op.to), use(type);
            }
            return uid;
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

        std::string StellarLikeTransactionDatabaseHelper::createOperationUid(const std::string &accountUid,
                                                                             const std::string &hash) {
            return SHA256::stringToHexHash(fmt::format("{}::{}", accountUid, hash));
        }

        bool StellarLikeTransactionDatabaseHelper::operationExists(soci::session &sql, const std::string &uid) {
            auto count = 0;
            sql << "SELECT COUNT(*) FROM stellar_operations WHERE uid = :uid", use(uid), into(count);
            return count >= 1;
        }
    }
}