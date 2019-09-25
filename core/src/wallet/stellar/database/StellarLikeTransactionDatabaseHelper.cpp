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
                auto ledger = fmt::format("{}", tx.ledger);
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
            Option<std::string> sourceAssetUid, sourceAssetCode, sourceAssetIssuer, sourceAmount;
            Option<std::string> assetCode, assetIssuer;
            int type, successful;
            std::string amount;
            sql << "SELECT o.hash, t.successful, o.type, o.amount, o.from_address, o.to_address,"
                   " o.created_at, o.source_amount, a.asset_type, a.asset_code, a.asset_issuer,"
                   " o.source_asset_uid, t.hash "
                   "FROM stellar_account_operations AS ao "
                   "LEFT JOIN stellar_operations AS o ON ao.operation_uid = o.uid "
                   "LEFT JOIN stellar_transactions AS t ON t.uid = o.transaction_uid "
                   "LEFT JOIN stellar_assets AS a ON  a.uid = o.asset_uid "
                   "WHERE ao.uid = :uid", use(accountOperationUid), into(out.id)
                   ,into(successful), into(type), into(amount), into(out.from)
                   ,into(out.to), into(out.createdAt), into(sourceAmount), into(out.asset.type)
                   ,into(assetCode), into(assetIssuer), into(sourceAssetUid)
                   ,into(out.transactionHash);

            out.amount = BigInt::fromString(amount);
            out.type = (stellar::OperationType)type;
            out.transactionSuccessful = successful == 1;
            out.asset.issuer = assetIssuer.getValueOr("");
            out.asset.code = assetCode.getValueOr("");
            out.sourceAmount = sourceAmount.map<BigInt>([] (const std::string& s) {
                return BigInt::fromString(s);
            });
            if (sourceAssetUid.nonEmpty()) {
                stellar::Asset asset;
                sql << "SELECT asset_type, asset_code, asset_issuer "
                       "FROM stellar_assets WHERE uid = :uid", use(sourceAssetUid.getValue())
                       ,into(asset.type), into(sourceAssetCode), into(sourceAssetCode);
                asset.code = sourceAssetCode.getValueOr("");
                asset.issuer = sourceAssetIssuer.getValueOr("");
                out.sourceAsset = asset;
            }
            return !out.transactionHash.empty();
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

        int StellarLikeTransactionDatabaseHelper::countOperationsForTransaction(soci::session &sql,
                                                                                const std::string &txHash,
                                                                                const std::string &senderAddress) {
            int count;
            sql << "SELECT COUNT(*) FROM stellar_operations "
                   "WHERE transaction_hash = :hash AND from_address = :address"
                   , use(txHash), use(senderAddress), into(count);
            return count;
        }
    }
}