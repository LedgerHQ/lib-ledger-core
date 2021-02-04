/*
 *
 * RippleLikeOperationDatabaseHelper.cpp
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

#include "RippleLikeOperationDatabaseHelper.hpp"
#include <database/PreparedStatement.hpp>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/ripple/database/RippleLikeTransactionDatabaseHelper.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <database/soci-number.h>

using namespace soci;

namespace {
    using namespace ledger::core;

    // Transaction
    struct TransactionBinding {
        std::vector<std::string> uid;
        std::vector<std::string> hash;
        std::vector<std::string> value;
        std::vector<Option<std::string>> blockUid;
        std::vector<std::chrono::system_clock::time_point> time;
        std::vector<std::string> sender;
        std::vector<std::string> receiver;
        std::vector<std::string> fees;
        std::vector<uint64_t> confirmations;
        std::vector<BigInt> sequence;
        std::vector<Option<uint64_t>> tag;
        std::vector<int32_t> status;

        void update(const std::string& txUid, const Option<std::string>& bUid,
                const RippleLikeBlockchainExplorerTransaction& tx) {
            uid.push_back(txUid);
            hash.push_back(tx.hash);
            value.push_back(tx.value.toHexString());
            blockUid.push_back(bUid);
            time.push_back(tx.receivedAt);
            sender.push_back(tx.sender);
            receiver.push_back(tx.receiver);
            fees.push_back(tx.fees.toHexString());
            confirmations.push_back(tx.confirmations);
            sequence.push_back(tx.sequence);
            tag.push_back(tx.destinationTag);
            status.push_back(tx.status);
        }
    };
    const auto UPSERT_TRANSACTION = db::stmt<TransactionBinding>(
        "INSERT INTO ripple_transactions VALUES("
        ":transaction_uid, :hash, :value, :block_uid, :time, :sender,"
        ":receiver, :fees, :confirmations, :sequence, :tag, :status"
        ") ON CONFLICT(transaction_uid) DO UPDATE SET block_uid = :block_uid, status = :status",
        [] (auto& s, auto& b) {
            s,
            use(b.uid, "transaction_uid"),
            use(b.hash, "hash"),
            use(b.value, "value"),
            use(b.blockUid, "block_uid"),
            use(b.time, "time"),
            use(b.sender, "sender"),
            use(b.receiver, "receiver"),
            use(b.fees, "fees"),
            use(b.confirmations, "confirmations"),
            use(b.sequence, "sequence"),
            use(b.tag, "tag"),
            use(b.status, "status");
        }
    );

    // Ripple Memo
    struct MemoBinding {
        std::vector<std::string> txUid;
        std::vector<std::string> data;
        std::vector<std::string> mfmt;
        std::vector<std::string> ty;
        std::vector<int> index;

        void update(const RippleLikeBlockchainExplorerTransaction& tx) {
            auto i = 0;
            for (const auto& memo : tx.memos) {
                txUid.push_back(tx.hash);
                data.push_back(memo.data);
                mfmt.push_back(memo.fmt);
                ty.push_back(memo.ty);
                index.push_back(i);
                i += 1;
            }
        }
    };
    const auto UPSERT_MEMO = db::stmt<MemoBinding>(
            "INSERT INTO ripple_memos VALUES ("
            ":tx_uid, :data, :fmt, :ty, :i"
            ") ON CONFLICT DO NOTHING",
            [] (auto& s, auto& b) {
                s, use(b.txUid), use(b.data), use(b.mfmt), use(b.ty), use(b.index);
            }
    );

    // Operation
    struct RippleOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> txUid;
        std::vector<std::string> txHash;

        void update(const std::string& opUid, const std::string& tUid, const std::string& tHash) {
            uid.push_back(opUid);
            txUid.push_back(tUid);
            txHash.push_back(tHash);
        }
    };
    const auto UPSERT_RIPPLE_OPERATION = db::stmt<RippleOperationBinding>(
        "INSERT INTO ripple_operations VALUES("
        ":uid, :transaction_uid, :transaction_hash"
        ") ON CONFLICT DO NOTHING",
        [] (auto& s, auto& b) {
            s, use(b.uid), use(b.txUid), use(b.txHash);
        }
    );
}

namespace ledger {
    namespace core {

        void RippleLikeOperationDatabaseHelper::bulkInsert(soci::session &sql, const std::vector<Operation> &ops) {
            if (ops.empty())
                return ;
            PreparedStatement<OperationBinding> opStmt;
            PreparedStatement<BlockBinding> blockStmt;
            PreparedStatement<TransactionBinding> txStmt;
            PreparedStatement<RippleOperationBinding> rippleOpStmt;
            PreparedStatement<MemoBinding> memoStmt;

            BulkInsertDatabaseHelper::UPSERT_OPERATION(sql, opStmt);
            BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
            UPSERT_TRANSACTION(sql, txStmt);
            UPSERT_RIPPLE_OPERATION(sql, rippleOpStmt);
            UPSERT_MEMO(sql, memoStmt);

            for (const auto& op : ops) {
                const auto& tx = op.rippleTransaction.getValue();
                if (op.block.nonEmpty()) {
                    blockStmt.bindings.update(op.block.getValue());
                }
                auto blockUid = op.block.map<std::string>([] (const auto& b) {
                    return b.getUid();
                });
                auto txUid = RippleLikeTransactionDatabaseHelper::createRippleTransactionUid(op.accountUid, tx.hash);
                txStmt.bindings.update(txUid, blockUid, tx);
                memoStmt.bindings.update(tx);
                rippleOpStmt.bindings.update(op.uid, txUid, tx.hash);
                opStmt.bindings.update(op);
            }
            if (!blockStmt.bindings.hash.empty())
                blockStmt.execute();
            txStmt.execute();
            opStmt.execute();
            rippleOpStmt.execute();
        }
    }
}