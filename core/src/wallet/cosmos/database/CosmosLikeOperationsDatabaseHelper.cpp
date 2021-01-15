/*
 *
 * TezosLikeOperationDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/01/2021.
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

#include "CosmosLikeOperationsDatabaseHelper.hpp"
#include <database/PreparedStatement.hpp>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/BigInt.hpp>
#include <crypto/SHA256.hpp>
#include <unordered_set>
#include <debug/Benchmarker.h>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/cosmos/database/SociCosmosAmount.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>



using namespace soci;

namespace {
    using namespace ledger::core;

    // Cosmos operations
    struct CosmosOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> msgUid;

        void update(const std::string& op, const std::string& msg) {
            uid.push_back(op);
            msgUid.push_back(msg);
        }

        void clear() {
            uid.clear();
            msgUid.clear();
        }
    };

    const auto UPSERT_COSMOS_OPERATION = db::stmt<CosmosOperationBinding>(
            "INSERT INTO cosmos_operations VALUES(:uid, :message_uid) "
            "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, use(b.uid), use(b.msgUid);
            });

   // Transaction
    struct TransactionBinding {
        std::vector<std::string> uid; 
        std::vector<std::string> hash; 
        std::vector<Option<std::string>> blockUid; 
        std::vector<std::string> date;
        std::vector<std::string> fee;
        std::vector<std::string> gas; 
        std::vector<Option<std::string>> gasUsed; 
        std::vector<std::string> memo;
        
        void update(const cosmos::Transaction& tx) {
            Option<std::string> txblockUid;
            if (tx.block.nonEmpty() && !tx.block.getValue().hash.empty()) {
                txblockUid = BlockDatabaseHelper::createBlockUid(tx.block.getValue());
            }
            auto txdate = DateUtils::toJSON(tx.timestamp);
            auto txfee = soci::coinsToString(tx.fee.amount);
            auto txgas = tx.fee.gas.toString();
            auto txgasUsed = tx.gasUsed.flatMap<std::string>([](const BigInt &g) {
                return g.toString();
            });

            uid.push_back(tx.uid);
            hash.push_back(tx.hash);
            blockUid.push_back(txblockUid);
            date.push_back(txdate);
            fee.push_back(txfee);
            gas.push_back(txgas);
            gasUsed.push_back(txgasUsed);
            memo.push_back(tx.memo);
        }

        void clear() {
            uid.clear();
            hash.clear();
            blockUid.clear();
            date.clear();
            fee.clear();
            gas.clear();
            gasUsed.clear();
            memo.clear();
        }
    };

    const auto UPSERT_TRANSACTION = db::stmt<TransactionBinding>(
        "INSERT INTO cosmos_transactions("
        "uid, hash, block_uid, time, fee_amount, gas, gas_used, memo"
        ") VALUES (:uid, :hash, :block_uid, :time, :fee, :gas, :gas_used, :memo)"
        " ON CONFLICT(uid) DO UPDATE SET block_uid = :buid, gas_used = :gused",
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid), 
                use(b.hash), 
                use(b.blockUid), 
                use(b.date), 
                use(b.fee),
                use(b.gas), 
                use(b.gasUsed), 
                use(b.memo),
                use(b.blockUid), 
                use(b.gasUsed);          
            });
}
namespace ledger {
    namespace core {
        void CosmosLikeOperationsDatabaseHelper::bulkInsert(soci::session &sql,
                const std::vector<CosmosLikeOperation> &operations) {
            if (operations.empty())
                return;
            Benchmarker rawInsert("raw_db_insert_cosmos", nullptr);
            rawInsert.start();
            PreparedStatement<OperationBinding> operationStmt;
            PreparedStatement<BlockBinding> blockStmt;
            PreparedStatement<CosmosOperationBinding> cosmosOpStmt;
            PreparedStatement<TransactionBinding> transactionStmt;

            BulkInsertDatabaseHelper::UPSERT_OPERATION(sql, operationStmt);
            BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
            UPSERT_COSMOS_OPERATION(sql, cosmosOpStmt);
            UPSERT_TRANSACTION(sql, transactionStmt);

            for (auto& op : operations) {
                const auto& tx = std::static_pointer_cast<CosmosLikeTransactionApi>(op.getTransaction())->getRawData();
                
                // Upsert block
                if (tx.block.nonEmpty()) {
                    blockStmt.bindings.update(tx.block.getValue());
                }
                // Upsert operation
                operationStmt.bindings.update(op);
                // Upsert transaction
                
                transactionStmt.bindings.update(tx);
                // cosmos operation
                const auto msg = std::static_pointer_cast<CosmosLikeMessage>(op.getMessage());
                cosmosOpStmt.bindings.update(op.uid, msg->getRawData().uid);
            }
            
            // 1- block
            if (!blockStmt.bindings.uid.empty()) {
                blockStmt.execute();
            }

            // 2- cosmos_transaction 
            transactionStmt.execute();

            // 3- operations
            operationStmt.execute();
            
            // 4- cosmos_operations
            cosmosOpStmt.execute();

            rawInsert.stop();
        }
    }
}