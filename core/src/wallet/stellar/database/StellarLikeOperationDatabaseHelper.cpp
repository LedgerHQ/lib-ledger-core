/*
 *
 * StellarLikeOperationDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Habib LAFET on 20/01/2021.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ledger
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

#include "StellarLikeOperationDatabaseHelper.hpp"
#include <database/PreparedStatement.hpp>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/BigInt.hpp>
#include <database/soci-backend-utils.h>
#include <debug/Benchmarker.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/stellar/database/StellarLikeTransactionDatabaseHelper.hpp>
#include <wallet/stellar/database/StellarLikeAssetDatabaseHelper.hpp>

using namespace soci;

namespace {
    using namespace ledger::core;
    // Transaction
    struct TransactionBinding {
        std::vector<std::string> uid;
        std::vector<std::string> hash;
        std::vector<std::string> sourceAccount;
        std::vector<std::string> sequence;
        std::vector<std::string> fee;
        std::vector<int> successful;
        std::vector<std::string> ledger;
        std::vector<std::string> memoType;
        std::vector<std::string> memo;

        void update(const stellar::Transaction& tx, const std::string& txUid) {
            const auto feePaid = tx.feePaid.toString();
            const auto seq = tx.sourceAccountSequence.toString();
            const auto success = tx.successful ? 1 : 0;
            
            uid.push_back(txUid);
            hash.push_back(tx.hash);
            sourceAccount.push_back(tx.sourceAccount);
            sequence.push_back(seq);
            fee.push_back(feePaid);
            successful.push_back(success);
            ledger.push_back(fmt::format("{}", tx.ledger));
            memoType.push_back(tx.memoType);
            memo.push_back(tx.memo);
        }

        void clear() {
            uid.clear();
            hash.clear();
            sourceAccount.clear();
            sequence.clear();
            fee.clear();
            successful.clear();
            ledger.clear();
            memoType.clear();
            memo.clear();
        }
    };
    const auto UPSERT_TRANSACTION = db::stmt<TransactionBinding>(
        "INSERT INTO stellar_transactions VALUES(:uid, :hash, :account, :sequence, :fee, :success, :ledger,"
        ":memo_type, :memo) "
        "ON CONFLICT DO NOTHING",
        [] (auto& s, auto&  b) {
            s, use(b.uid), use(b.hash), use(b.sourceAccount), use(b.sequence), use(b.fee),
            use(b.successful), use(b.ledger), use(b.memoType), use(b.memo);
        });

    // Assets
    struct AssetBinding {
        std::vector<std::string> uid;
        std::vector<std::string> type;
        std::vector<Option<std::string>> code;
        std::vector<Option<std::string>> issuer;

        void update(const stellar::Asset &asset) { 
            Option<std::string> assetCode;
            Option<std::string> assetIssuer;

            if (!asset.code.empty())
                assetCode = asset.code;
            if (!asset.issuer.empty())
                assetIssuer = asset.issuer;

            const auto assetUid = StellarLikeAssetDatabaseHelper::createAssetUid(asset.type, assetCode, assetIssuer);  
            uid.push_back(assetUid);
            type.push_back(asset.type);
            code.push_back(assetCode);
            issuer.push_back(assetIssuer);
        }

        void clear() {
            uid.clear();
            type.clear();
            code.clear();
            issuer.clear();
        }
    };
    const auto UPSERT_ASSERT = db::stmt<AssetBinding>(
        "INSERT INTO stellar_assets VALUES (:uid, :type, :code, :issuer) "
        "ON CONFLICT DO NOTHING",
        [] (auto& s, auto&  b) {
            s, use(b.uid), use(b.type), use(b.code), use(b.issuer);
        });

    // operations
    struct StellarOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> txUid;
        std::vector<std::string> id;
        std::vector<std::chrono::system_clock::time_point> createdAt;
        std::vector<std::string> assetUid;
        std::vector<Option<std::string>> sourceAssetUid;
        std::vector<std::string> amount;
        std::vector<Option<std::string>> srcAmount;
        std::vector<std::string> from;
        std::vector<std::string> to;
        std::vector<int> type;

        void update(const std::string &accountUid, const std::string& currencyName, const stellar::Operation &op) {
            auto opUid = StellarLikeTransactionDatabaseHelper::createOperationUid(accountUid, op.id);
            auto hash = StellarLikeTransactionDatabaseHelper::createTransactionUid(currencyName, op.transactionHash);
            auto auid = StellarLikeAssetDatabaseHelper::createAssetUid(op.asset);
            auto sourceAuid = op.sourceAsset.map<std::string>([] (const stellar::Asset& asset) {
                return StellarLikeAssetDatabaseHelper::createAssetUid(asset);
            });
            auto opAmount = op.amount.toString();
            auto opSourceAmount = op.sourceAmount.map<std::string>([] (const BigInt& b) {
                return b.toString();
            });
            int opType = static_cast<int>(op.type);
            
            uid.push_back(opUid);
            txUid.push_back(hash);
            id.push_back(op.id);
            createdAt.push_back(op.createdAt);
            assetUid.push_back(auid);
            sourceAssetUid.push_back(sourceAuid);
            amount.push_back(opAmount);
            srcAmount.push_back(opSourceAmount);
            from.push_back(op.from);
            to.push_back(op.to);
            type.push_back(opType);
        }

        void clear() {
            uid.clear();
            txUid.clear();
            id.clear();
            createdAt.clear();
            assetUid.clear();
            sourceAssetUid.clear();
            amount.clear();
            srcAmount.clear();
            from.clear();
            to.clear();
            type.clear();
        }
    };
    const auto UPSERT_STELLAR_OPERATION = db::stmt<StellarOperationBinding>(
        "INSERT INTO stellar_operations VALUES("
        ":uid, :tx_uid, :hash, :time, :asset_uid, :src_asset_uid, :amount, :src_amount, :from, :to, :type"
        ") "
        "ON CONFLICT DO NOTHING",
        [] (auto& s, auto&  b) {
            s, 
            use(b.uid), 
            use(b.txUid), 
            use(b.id), 
            use(b.createdAt), 
            use(b.assetUid), 
            use(b.sourceAssetUid),
            use(b.amount), 
            use(b.srcAmount), 
            use(b.from), 
            use(b.to), 
            use(b.type);
        });
    

    // stellar_account_operations
    struct StellarAccountOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> opUid;

        void update(const Operation &operation) {
            auto& operationValue = operation.stellarOperation.getValue().operation;
            auto stellarOpId = StellarLikeTransactionDatabaseHelper::createOperationUid(operation.accountUid, operationValue.id);       
            uid.push_back(operation.uid);
            opUid.push_back(stellarOpId);
        }

        void clear() {
            uid.clear();
            opUid.clear();
        }
    };
    const auto UPSERT_STELLAR_ACCOUNT_OPERATION = db::stmt<StellarAccountOperationBinding>(
        "INSERT INTO stellar_account_operations VALUES(:uid, :op_uid) "
        "ON CONFLICT DO NOTHING",
        [] (auto& s, auto&  b) {
            s, 
            use(b.uid), 
            use(b.opUid);
        });


}
namespace ledger {
    namespace core {

        void StellarLikeOperationDatabaseHelper::bulkInsert(soci::session &sql,
                const std::vector<Operation> &operations) {
            if (operations.empty())
                return;
            Benchmarker rawInsert("raw_db_insert_stellar", nullptr);
            rawInsert.start();
            PreparedStatement<OperationBinding> operationStmt;
            PreparedStatement<BlockBinding> blockStmt;
            PreparedStatement<TransactionBinding> transactionStmt;
            PreparedStatement<AssetBinding> assetStmt;
            PreparedStatement<StellarOperationBinding> stellarOperationStmt;
            PreparedStatement<StellarAccountOperationBinding> stellarAccountOperationStmt;
            
            BulkInsertDatabaseHelper::UPSERT_OPERATION(sql, operationStmt);
            BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
            UPSERT_TRANSACTION(sql, transactionStmt);
            UPSERT_ASSERT(sql, assetStmt);
            UPSERT_STELLAR_OPERATION(sql, stellarOperationStmt);
            UPSERT_STELLAR_ACCOUNT_OPERATION(sql, stellarAccountOperationStmt);
                   
            for (const auto& op : operations) {
                if (op.block.hasValue()) {
                    blockStmt.bindings.update(op.block.getValue());
                }
                // Upsert operation
                operationStmt.bindings.update(op);
                // Upsert transaction
                auto& tx = op.stellarOperation.getValue().transaction;
                const auto txUid = StellarLikeTransactionDatabaseHelper::createTransactionUid(op.currencyName, tx.hash);
                transactionStmt.bindings.update(tx, txUid); 
                // Upsert assets
                auto& stOp = op.stellarOperation.getValue().operation;
                assetStmt.bindings.update(stOp.asset);
                if (stOp.sourceAsset)  {
                    assetStmt.bindings.update(stOp.sourceAsset.getValue());
                }
                // Upsert stellar operations
                stellarOperationStmt.bindings.update(op.accountUid, op.currencyName, stOp);
                // Upsert stellar operations
                stellarAccountOperationStmt.bindings.update(op);
            }
            // block
            if (!blockStmt.bindings.uid.empty())
                blockStmt.execute();
            // transactions
            transactionStmt.execute();
            // assets
            assetStmt.execute();
            // operations
            operationStmt.execute();
            // stellar operations
            stellarOperationStmt.execute(); 
            //stellar accounts operations
            stellarAccountOperationStmt.execute();
            
            rawInsert.stop();
        }
    }
}