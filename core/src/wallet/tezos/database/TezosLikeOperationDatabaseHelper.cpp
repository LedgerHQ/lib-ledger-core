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

#include "TezosLikeOperationDatabaseHelper.hpp"
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

using namespace soci;

namespace {
    using namespace ledger::core;

    // Tezos operations
    struct TezosOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> txUid;
        std::vector<std::string> txHash;

        void update(const std::string& opUid, const std::string& transactionUid,
                const std::string& transactionHash) {
            uid.push_back(opUid);
            txUid.push_back(transactionUid);
            txHash.push_back(transactionHash);
        }

        void clear() {
            uid.clear();
            txUid.clear();
            txHash.clear();
        }
    };

    const auto UPSERT_TEZOS_OPERATION = db::stmt<TezosOperationBinding>(
            "INSERT INTO tezos_operations VALUES(:uid, :tx_uid, :tx_hash) "
            "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, use(b.uid), use(b.txUid), use(b.txHash);
            });

    // Tezos originated operations
    struct TezosOriginatedOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> txUid;
        std::vector<std::string> originatedAccountUid;

        void update(const std::string& opUid, const std::string& transactionUid,
                const std::string& origAccountUid) {
            uid.push_back(opUid);
            txUid.push_back(transactionUid);
            originatedAccountUid.push_back(origAccountUid);
        }

        void clear() {
            uid.clear();
            txUid.clear();
            originatedAccountUid.clear();
        }
    };

    const auto UPSERT_TEZOS_ORIGINATED_OPERATION = db::stmt<TezosOriginatedOperationBinding>(
            "INSERT INTO tezos_originated_operations VALUES(:uid, :transaction_uid, :originated_account_uid)"
            "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, use(b.uid), use(b.txUid), use(b.originatedAccountUid);
            });

    //tezos originated account
    struct TezosOriginatedAccountBinding {
        std::vector<std::string> uid;
        std::vector<std::string> accountUid;
        std::vector<std::string> address;
        std::vector<int> spendable;
        std::vector<int> delegatable;
        std::vector<std::string> pubKey;

        void update(const Operation &operation) {
            auto transaction = operation.tezosTransaction.getValue();

            if (transaction.originatedAccountUid.empty()) { //insertion
                auto origAccount = transaction.originatedAccount.getValue();
                uid.push_back(TezosLikeAccountDatabaseHelper::createOriginatedAccountUid(operation.accountUid, origAccount.address));
                address.push_back(origAccount.address);
                spendable.push_back(static_cast<int>(origAccount.spendable));
                delegatable.push_back(static_cast<int>(origAccount.delegatable));
            }
            else { //update: only pubkey will be updated
                uid.push_back(transaction.originatedAccountUid);
                address.push_back("");
                spendable.push_back(false);
                delegatable.push_back(false);
            }
         
            accountUid.push_back(operation.accountUid);
            pubKey.push_back(transaction.publicKey.getValueOr(""));
        }

        void clear() {
            uid.clear();
            accountUid.clear();
            address.clear();
            spendable.clear();
            delegatable.clear();
            pubKey.clear();
        }
    };

    const auto UPSERT_TEZOS_ORIGINATED_ACCOUNT = db::stmt<TezosOriginatedAccountBinding>(
            "INSERT INTO tezos_originated_accounts VALUES("
            ":uid, :tezos_account_uid, :address, :spendable, :delegatable, :public_key)"
            "ON CONFLICT(uid) DO UPDATE SET public_key = :pub_key",
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid),
                use(b.accountUid),
                use(b.address),
                use(b.spendable),
                use(b.delegatable),
                use(b.pubKey),
                use(b.pubKey);
            });

    // Transaction
    struct TransactionBinding {
        std::vector<std::string> uid;
        std::vector<std::string> hash;
        std::vector<std::string> value;
        std::vector<std::string> blockUid;
        std::vector<std::chrono::system_clock::time_point> time;
        std::vector<std::string> sender;
        std::vector<std::string> receiver;
        std::vector<std::string> fees;
        std::vector<std::string> gasLimit;
        std::vector<std::string> storageLimit;
        std::vector<uint64_t> confirmations;
        std::vector<std::string> type;
        std::vector<std::string> publicKey;
        std::vector<std::string> originatedAccount;
        std::vector<uint64_t> status;
        
        void update(const TezosLikeBlockchainExplorerTransaction& tx, const std::string& txUid) {
            uid.push_back(txUid);
            hash.push_back(tx.hash);
            value.push_back(tx.value.toHexString());
            //blockUid.push_back(tx.block.map<std::string>([] (const TezosLikeBlockchainExplorer::Block& block) {
            //    return block.getUid();
            //}));
            blockUid.push_back(tx.block.hasValue() ? tx.block->getUid() : "");
            time.push_back(tx.receivedAt);
            sender.push_back(tx.sender);
            receiver.push_back(tx.receiver);
            fees.push_back(tx.fees.toHexString());
            gasLimit.push_back(tx.gas_limit.toHexString());
            storageLimit.push_back(tx.storage_limit.toHexString());
            confirmations.push_back(tx.confirmations);
            type.push_back(api::to_string(tx.type));
            publicKey.push_back(tx.publicKey.getValueOr(""));
            std::string sOrigAccount;
                if (tx.originatedAccount.hasValue()) {
                    std::stringstream origAccount;
                    std::vector<std::string> vOrigAccount{tx.originatedAccount.getValue().address, std::to_string(tx.originatedAccount.getValue().spendable), std::to_string(tx.originatedAccount.getValue().delegatable)};
                    strings::join(vOrigAccount, origAccount, ":");
                    sOrigAccount = origAccount.str();
                }
            originatedAccount.push_back(sOrigAccount);
            status.push_back(tx.status);
            
        }

        void clear() {
            uid.clear();
            hash.clear();
            value.clear();
            blockUid.clear();
            time.clear();
            sender.clear();
            receiver.clear();
            fees.clear();
            gasLimit.clear();
            storageLimit.clear();
            confirmations.clear();
            type.clear();
            publicKey.clear();
            originatedAccount.clear();
            status.clear();
        }
    };

    const auto UPSERT_TRANSACTION = db::stmt<TransactionBinding>(
            "INSERT INTO tezos_transactions VALUES("
            " :tx_uid, :hash, :value, :block_uid, :time, :sender, :receiver, :fees, :gas_limit, "
            " :storage_limit, :confirmations, :type, :public_key, :originated_account, :status)"
            " ON CONFLICT(transaction_uid) DO UPDATE SET block_uid = :block, status = :code", [] (auto& s, auto&  b) {
                s, 
                use(b.uid), 
                use(b.hash),
                use(b.value),
                use(b.blockUid), 
                use(b.time),
                use(b.sender),
                use(b.receiver),
                use(b.fees), 
                use(b.gasLimit),
                use(b.storageLimit),
                use(b.confirmations),
                use(b.type), 
                use(b.publicKey),
                use(b.originatedAccount),
                use(b.status),
                use(b.blockUid), 
                use(b.status);
            });
}

namespace ledger {
    namespace core {

        void TezosLikeOperationDatabaseHelper::bulkInsert(soci::session &sql,
                const std::vector<Operation> &operations) {
            if (operations.empty())
                return;
            Benchmarker rawInsert("raw_db_insert_tezos", nullptr);
            rawInsert.start();
            PreparedStatement<OperationBinding> operationStmt;
            PreparedStatement<BlockBinding> blockStmt;
            PreparedStatement<TezosOperationBinding> tezosOpStmt;
            PreparedStatement<TransactionBinding> transactionStmt;
            PreparedStatement<TezosOriginatedAccountBinding> tezosOrigAccountStmt;
            PreparedStatement<TezosOriginatedOperationBinding> tezosOrigOpStmt;

            BulkInsertDatabaseHelper::UPSERT_OPERATION(sql, operationStmt);
            BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
            UPSERT_TEZOS_OPERATION(sql, tezosOpStmt);
            UPSERT_TRANSACTION(sql, transactionStmt);
            UPSERT_TEZOS_ORIGINATED_ACCOUNT(sql, tezosOrigAccountStmt);
            UPSERT_TEZOS_ORIGINATED_OPERATION(sql, tezosOrigOpStmt);

            for (const auto& op : operations) {
                // Upsert block
                if (op.block.hasValue()) {
                    blockStmt.bindings.update(op.block.getValue());
                }
                // Upsert operation
                operationStmt.bindings.update(op);
                // Upsert transaction
                auto& tx = op.tezosTransaction.getValue();
                const auto txUid =  TezosLikeTransactionDatabaseHelper::createTezosTransactionUid(op.accountUid, tx.hash, tx.type);
                transactionStmt.bindings.update(tx, txUid);
                // tezos operation
                tezosOpStmt.bindings.update(op.uid, txUid, tx.hash);

                if (!tx.originatedAccountUid.empty()) {               
                    if (tx.type == api::TezosOperationTag::OPERATION_TAG_REVEAL && tx.publicKey.hasValue()) {
                        tezosOrigAccountStmt.bindings.update(op);
                    } 
                    tezosOrigOpStmt.bindings.update(op.uid, txUid, tx.originatedAccountUid);
                }
                else {
                    if (op.type == api::OperationType::SEND) {
                        if (tx.type == api::TezosOperationTag::OPERATION_TAG_ORIGINATION && tx.status == 1) {
                            tezosOrigAccountStmt.bindings.update(op);
                        }
                    }
                }
            }
            
            // 1- Bulk insert block (dependency for operation and tezos transaction)
            if (!blockStmt.bindings.uid.empty()) {
                blockStmt.execute();
            }

            // 2- Bulk insert tezos_transaction 
            transactionStmt.execute();

            // 3- Bulk insert operations (dependency of  tezos_operations)
            operationStmt.execute();
            
            // 4- Bulk insert tezos_operations
            tezosOpStmt.execute();

            // 5- Bulk insert tezos_originated_accounts
            if (!tezosOrigAccountStmt.bindings.uid.empty()) {
                tezosOrigAccountStmt.execute();
            }

            // 6- Bulk insert tezos_originated_operations
            if (!tezosOrigOpStmt.bindings.uid.empty()) {
                tezosOrigOpStmt.execute();
            }

            rawInsert.stop();
        }
    }
}