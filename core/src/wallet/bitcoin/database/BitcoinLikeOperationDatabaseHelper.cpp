/*
 *
 * BitcoinLikeOperationDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 30/12/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "BitcoinLikeOperationDatabaseHelper.hpp"
#include <database/PreparedStatement.hpp>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/BigInt.hpp>
#include <crypto/SHA256.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <unordered_set>
#include <database/soci-backend-utils.h>
#include <debug/Benchmarker.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>

using namespace soci;

namespace {
    using namespace ledger::core;

    // Bitcoin operations
    struct BitcoinOperationBinding {
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

    const auto UPSERT_BITCOIN_OPERATION = db::stmt<BitcoinOperationBinding>(
            "INSERT INTO bitcoin_operations VALUES(:uid, :tx_uid, :tx_hash) "
            "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, use(b.uid), use(b.txUid), use(b.txHash);
            });

    // Transaction
    struct TransactionBinding {
        std::vector<std::string> uid;
        std::vector<std::string> hash;
        std::vector<uint32_t> version;
        std::vector<std::chrono::system_clock::time_point> date;
        std::vector<uint64_t> lockTime;
        std::vector<Option<std::string>> blockUid;

        void update(const BitcoinLikeBlockchainExplorerTransaction& tx, const std::string& txUid) {
            uid.push_back(txUid);
            hash.push_back(tx.hash);
            version.push_back(tx.version);
            date.push_back(tx.receivedAt);
            lockTime.push_back(tx.lockTime);
            blockUid.push_back(tx.block.map<std::string>([] (const BitcoinLikeBlockchainExplorer::Block& block) {
                return block.getUid();
            }));
        }

        void clear() {
            uid.clear();
            hash.clear();
            version.clear();
            date.clear();
            lockTime.clear();
            blockUid.clear();
        }
    };

    const auto UPSERT_TRANSACTION = db::stmt<TransactionBinding>(
            "INSERT INTO bitcoin_transactions VALUES("
            ":tx_uid, :hash, :version, :block_uid, :time, :locktime)"
            " ON CONFLICT(transaction_uid) DO UPDATE SET block_uid = :block_uid", [] (auto& s, auto&  b) {
                s, use(b.uid, "tx_uid"), use(b.hash, "hash"),
                use(b.version, "version"),
                use(b.blockUid, "block_uid"), use(b.date, "time"),
                use(b.lockTime, "locktime");
            });

    // Input
    struct InputBinding {
        std::vector<BitcoinLikeBlockchainExplorerInput> input;
        std::vector<std::string> uid;
        std::vector<std::string> prevBtcTxUid;
        std::vector<Option<uint64_t>> amount;
        std::vector<Option<uint32_t>> previousTxOutputIndex;
        std::vector<Option<std::string>> previousTxHash;
        std::vector<Option<std::string>> address;
        std::vector<Option<std::string>> coinbase;
        std::vector<uint32_t> sequence;

        void update(const BitcoinLikeBlockchainExplorerInput& in, const std::string& inputUid,
                const std::string& previousTxUid,
                const std::string& accountUid) {
            uid.push_back(inputUid);
            input.push_back(in);
            amount.push_back(in.value.map<uint64_t>([] (const BigInt& v) {
                return v.toUint64();
            }));
            prevBtcTxUid.push_back(previousTxUid);
            previousTxOutputIndex.push_back(in.previousTxOutputIndex);
            previousTxHash.push_back(in.previousTxHash);
            address.push_back(in.address);
            coinbase.push_back(in.coinbase);
            sequence.push_back(in.sequence);
        }

        void clear() {
            input.clear();
            uid.clear();
            prevBtcTxUid.clear();
            amount.clear();
            previousTxOutputIndex.clear();
            previousTxHash.clear();
            address.clear();
            coinbase.clear();
            sequence.clear();
        }
    };

    const auto UPSERT_INPUT = db::stmt<InputBinding>(
            "INSERT INTO bitcoin_inputs VALUES(:uid, :idx, :hash, :prev_tx_uid, :amount, :address, "
            ":coinbase, :sequence) ON CONFLICT DO NOTHING", [] (auto& s, auto&  b) {
                s, use(b.uid), use(b.previousTxOutputIndex), use(b.previousTxHash),
                use(b.prevBtcTxUid), use(b.amount), use(b.address), use(b.coinbase),
                use(b.sequence);
            });

    // Transaction inputs
    struct TransactionInputBinding {
        std::vector<std::string> txUid;
        std::vector<std::string> txHash;
        std::vector<std::string> inputUid;
        std::vector<int> inputIdx;

        void update(const std::string& tUid, const std::string& tHash, const std::string& iUid,
                    int iIdx) {
            txUid.push_back(tUid);
            txHash.push_back(tHash);
            inputUid.push_back(iUid);
            inputIdx.push_back(iIdx);
        }

        void clear() {
            txUid.clear();
            txHash.clear();
            inputUid.clear();
            inputIdx.clear();
        }

    };

    const auto UPSERT_TRANSACTION_INPUT = db::stmt<TransactionInputBinding>(
            "INSERT INTO bitcoin_transaction_inputs VALUES(:tx_uid, :tx_hash, :input_uid, "
            ":input_idx) ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, use(b.txUid), use(b.txHash), use(b.inputUid), use(b.inputIdx);
            });

    // Output
    struct OutputBinding {
        std::vector<uint64_t> amount;
        std::vector<std::string> txUid;
        std::vector<std::string> txHash;
        std::vector<int> index;
        std::vector<std::string> script;
        std::vector<Option<std::string>> address;
        std::vector<Option<std::string>> accountUid;
        std::vector<Option<uint64_t>> blockHeight;
        std::vector<BitcoinLikeBlockchainExplorerOutput> output;
        std::vector<int> replaceable;

        void update(const BitcoinLikeBlockchainExplorerOutput& o, bool isReplaceable,
                const std::string& tUid, const std::string& tHash) {
            amount.push_back(o.value.toUint64());
            replaceable.push_back(isReplaceable ? 1 : 0);
            txUid.push_back(tUid);
            txHash.push_back(tHash);
            index.push_back(o.index);
            script.push_back(o.script);
            blockHeight.push_back(o.blockHeight);
            accountUid.push_back(o.accountUid);
            address.push_back(o.address);
        }

        void clear() {
            amount.clear();
            replaceable.clear();
            txUid.clear();
            txHash.clear();
            output.clear();
            script.clear();
            index.clear();
            blockHeight.clear();
            accountUid.clear();
            address.clear();
        }
    };

    const auto UPSERT_OUTPUT = db::stmt<OutputBinding>(
            "INSERT INTO bitcoin_outputs VALUES(:idx, :tx_uid, :hash, :amount, :script, :address, "
            ":account_uid, :block_height, :replaceable) "
            "ON CONFLICT DO NOTHING", [] (auto& s, auto&  b) {
                s, use(b.index), use(b.txUid), use(b.txHash), use(b.amount),
                use(b.script), use(b.address), use(b.accountUid),
                use(b.blockHeight), use(b.replaceable);
            });
}

namespace ledger {
    namespace core {

        void BitcoinLikeOperationDatabaseHelper::bulkInsert(soci::session &sql,
                const std::vector<Operation> &operations) {
            if (operations.empty())
                return;
            Benchmarker rawInsert("raw_db_insert", nullptr);
            rawInsert.start();
            PreparedStatement<OperationBinding> operationStmt;
            PreparedStatement<BlockBinding> blockStmt;
            PreparedStatement<BitcoinOperationBinding> bitcoinOpStmt;
            PreparedStatement<TransactionInputBinding> transactionInputStmt;
            PreparedStatement<TransactionBinding> transactionStmt;
            PreparedStatement<InputBinding> inputStmt;
            PreparedStatement<OutputBinding> outputStmt;

            BulkInsertDatabaseHelper::UPSERT_OPERATION(sql, operationStmt);
            BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
            UPSERT_BITCOIN_OPERATION(sql, bitcoinOpStmt);
            UPSERT_TRANSACTION(sql, transactionStmt);
            UPSERT_TRANSACTION_INPUT(sql, transactionInputStmt);
            UPSERT_INPUT(sql, inputStmt);
            UPSERT_OUTPUT(sql, outputStmt);

            for (const auto& op : operations) {
                if (op.block.hasValue()) {
                    blockStmt.bindings.update(op.block.getValue());
                }
                // Upsert operation
                operationStmt.bindings.update(op);
                // Upsert transaction
                auto& tx = op.bitcoinTransaction.getValue();
                const auto txUid =  BitcoinLikeTransactionDatabaseHelper::createBitcoinTransactionUid(op.accountUid, tx.hash);
                transactionStmt.bindings.update(tx, txUid);
                // Upsert Input
                bool replaceable = false;
                for (const auto& input : tx.inputs) {
                    replaceable = replaceable || (input.sequence < std::numeric_limits<uint32_t>::max());

                    /*
                     * In case transactions are issued with respect to zero knowledge protocol,
                     * previousTxHash is empty which causes conflict in bitcoin_inputs table
                     * Right now we generate a random 'hash' to compute inputUid, should be improved
                     * (e.g. use scriptSig of each input and sha256 it ...)
                    */
                    std::string hash;

                    //Returned by explorers when tx from zk protocol
                    std::string emptyPreviousTxHash = "0000000000000000000000000000000000000000000000000000000000000000";
                    auto previousTxHash = input.previousTxHash.getValueOr(emptyPreviousTxHash);
                    if (previousTxHash == emptyPreviousTxHash && input.signatureScript.nonEmpty()) {
                        previousTxHash =  SHA256::stringToHexHash(input.signatureScript.getValue());
                    }

                    auto inputUid = BitcoinLikeTransactionDatabaseHelper::createInputUid(op.accountUid,
                                              input.previousTxOutputIndex.getValueOr(0),
                                              previousTxHash,
                                              input.coinbase.getValueOr(""));

                    std::string prevBtcTxUid;
                    if (input.previousTxHash.nonEmpty() && input.previousTxHash.getValue() != emptyPreviousTxHash) {
                        prevBtcTxUid = BitcoinLikeTransactionDatabaseHelper::createBitcoinTransactionUid(op.accountUid, input.previousTxHash.getValue());
                    }
                    inputStmt.bindings.update(input, inputUid, prevBtcTxUid, op.accountUid);
                    transactionInputStmt.bindings.update(txUid, tx.hash, inputUid, input.index);
                }
                // Upsert output
                for (const auto& output : tx.outputs) {
                    outputStmt.bindings.update(output, replaceable && tx.block.isEmpty(), txUid, tx.hash);
                }
                // Bitcoin operation
                bitcoinOpStmt.bindings.update(op.uid, txUid, tx.hash);
            }
            // Bulk insert block (dependency for operation and bitcoin transaction)
            if (!blockStmt.bindings.uid.empty())
                blockStmt.execute();
            // Bulk insert transaction (dependency of bitcoin_input,
            // bitcoin_transaction_input and  bitcoin_output)
            transactionStmt.execute();
            // Bulk insert outputs
            outputStmt.execute();
            // Bulk insert bitcoin_input (dependency of bitcoin_transaction_inputs)
            inputStmt.execute();
            // Bulk insert  bitcoin_transaction_inputs
            transactionInputStmt.execute();
            // Bulk insert operations (dependency of  bitcoin operations)
            operationStmt.execute();
            // Bulk insert bitcoin operations
            bitcoinOpStmt.execute();

            rawInsert.stop();
        }
    }
}