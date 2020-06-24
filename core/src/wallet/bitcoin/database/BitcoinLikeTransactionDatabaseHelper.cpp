/*
 *
 * BitcoinLikeTransactionDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 02/06/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include <crypto/SHA256.hpp>
#include "BitcoinLikeTransactionDatabaseHelper.h"
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <database/soci-option.h>
#include <database/soci-date.h>
#include <database/soci-number.h>

#include <iostream>
using namespace std;

using namespace soci;

namespace ledger {
    namespace core {

        bool BitcoinLikeTransactionDatabaseHelper::transactionExists(soci::session &sql, const std::string &btcTxUid) {
            int32_t count = 0;
            sql << "SELECT COUNT(*) FROM bitcoin_transactions WHERE transaction_uid = :btcTxUid", use(btcTxUid), into(count);
            return count == 1;
        }

        std::string BitcoinLikeTransactionDatabaseHelper::createBitcoinTransactionUid(const std::string& accountUid, const std::string& txHash) {
            auto result = SHA256::stringToHexHash(fmt::format("uid:{}+{}", accountUid, txHash));
            return result;
        }

        std::string BitcoinLikeTransactionDatabaseHelper::putTransaction(soci::session &sql,
                                                                         const std::string &accountUid,
                                                                         const BitcoinLikeBlockchainExplorerTransaction &tx) {
            auto blockUid = tx.block.map<std::string>([] (const BitcoinLikeBlockchainExplorer::Block& block) {
                                   return block.getUid();
                               });

            auto btcTxUid = createBitcoinTransactionUid(accountUid, tx.hash);

            if (transactionExists(sql, btcTxUid)) {
                // UPDATE (we only update block information)
                if (tx.block.nonEmpty()) {
                    sql << "UPDATE bitcoin_transactions SET block_uid = :uid WHERE hash = :tx_hash",
                            use(blockUid), use(tx.hash);
                    auto blockHeight = tx.block.getValue().height;
                    sql << "UPDATE bitcoin_outputs SET block_height = :height WHERE transaction_hash = :tx_hash",
                    use(blockHeight), use(tx.hash);
                }
                return btcTxUid;
            } else {
                // Insert
                if (tx.block.nonEmpty()) {
                    BlockDatabaseHelper::putBlock(sql, tx.block.getValue());
                }
                sql << "INSERT INTO bitcoin_transactions VALUES("
                        ":tx_uid, :hash, :version, :block_uid, :time, :locktime"
                        ")",
                        use(btcTxUid),
                        use(tx.hash),
                        use(tx.version),
                        use(blockUid),
                        use(tx.receivedAt),
                        use(tx.lockTime);
                // Insert outputs
                for (const auto& output : tx.outputs) {
                    insertOutput(sql, btcTxUid, accountUid, tx.hash, output);
                }
                // Insert inputs
                for (const auto& input : tx.inputs) {
                    insertInput(sql, btcTxUid, accountUid, tx.hash, input);
                }
                return btcTxUid;
            }
        }

        void BitcoinLikeTransactionDatabaseHelper::insertOutput(soci::session &sql,
                                                                const std::string& btcTxUid,
                                                                const std::string &accountUid,
                                                                const std::string& transactionHash,
                                                                const BitcoinLikeBlockchainExplorerOutput &output) {
            auto value = output.value.toUint64();
            if (output.accountUid.hasValue() && output.accountUid.getValue() == accountUid) {
                sql << "INSERT INTO bitcoin_outputs VALUES(:idx, :tx_uid, :hash, :amount, :script, :address, :account_uid, :block_height)",
                        use(output.index), use(btcTxUid),
                        use(transactionHash), use(value),
                        use(output.script), use(output.address),
                        use(accountUid), use(output.blockHeight);
            } else {
                sql << "INSERT INTO bitcoin_outputs VALUES(:idx, :tx_uid, :hash, :amount, :script, :address, NULL, :block_height)",
                        use(output.index), use(btcTxUid),
                        use(transactionHash), use(value),
                        use(output.script), use(output.address),
                        use(output.blockHeight);
            }
        }

        void BitcoinLikeTransactionDatabaseHelper::insertInput(soci::session &sql,
                                                               const std::string& btcTxUid,
                                                               const std::string& accountUid,
                                                               const std::string& transactionHash,
                                                               const BitcoinLikeBlockchainExplorerInput &input) {
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

            auto uid = createInputUid(accountUid,
                                      input.previousTxOutputIndex.getValueOr(0),
                                      previousTxHash,
                                      input.coinbase.getValueOr(""));

            auto amount = input.value.map<uint64_t>([] (const BigInt& v) {
                return v.toUint64();
            });

            std::string prevBtcTxUid;
            if (input.previousTxHash.nonEmpty() && input.previousTxHash.getValue() != emptyPreviousTxHash) {
                prevBtcTxUid = createBitcoinTransactionUid(accountUid, input.previousTxHash.getValue());
            }

            sql << "INSERT INTO bitcoin_inputs VALUES(:uid, :idx, :hash, :prev_tx_uid, :amount, :address, :coinbase, :sequence)",
                    use(uid), use(input.previousTxOutputIndex), use(input.previousTxHash), use(prevBtcTxUid), use(amount),
                    use(input.address), use(input.coinbase), use(input.sequence);
            sql << "INSERT INTO bitcoin_transaction_inputs VALUES(:tx_uid, :tx_hash, :input_uid, :input_idx)",
                    use(btcTxUid), use(transactionHash), use(uid), use(input.index);
        }

        std::string BitcoinLikeTransactionDatabaseHelper::createInputUid(const std::string& accountUid,
                                                                         int32_t previousOutputIndex,
                                                                         const std::string &previousTxHash,
                                                                         const std::string &coinbase) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}+{}+{}", accountUid, previousOutputIndex, previousTxHash, coinbase));
        }

        bool BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(soci::session &sql,
                                                                        const std::string &hash,
                                                                        const std::string &accountUid,
                                                                        BitcoinLikeBlockchainExplorerTransaction &out) {
            rowset<row> rows = (sql.prepare <<
                    "SELECT  tx.hash, tx.version, tx.time, tx.locktime, "
                            "block.hash, block.height, block.time, block.currency_name "
                            "FROM bitcoin_transactions AS tx "
                            "LEFT JOIN blocks AS block ON tx.block_uid = block.uid "
                            "WHERE tx.hash = :hash", use(hash)
            );
            for (auto& row : rows) {
                inflateTransaction(sql, row, accountUid, out);
                return true;
            }
            return false;
        }

        bool BitcoinLikeTransactionDatabaseHelper::inflateTransaction(soci::session &sql,
                                                                      const soci::row &row,
                                                                      const std::string &accountUid,
                                                                      BitcoinLikeBlockchainExplorerTransaction &out) {
            out.hash = row.get<std::string>(0);
            out.version = (uint32_t) row.get<int32_t>(1);
            out.receivedAt = row.get<std::chrono::system_clock::time_point>(2);
            out.lockTime = (uint64_t) row.get<int>(3);
            if (row.get_indicator(4) != i_null) {
                BitcoinLikeBlockchainExplorer::Block block;
                block.hash = row.get<std::string>(4);
                block.height = get_number<uint64_t>(row, 5);
                block.time = row.get<std::chrono::system_clock::time_point>(6);
                block.currencyName = row.get<std::string>(7);
                out.block = block;
            }
            // Fetch inputs
            rowset<soci::row> inputRows = (sql.prepare <<
                "SELECT  ti.input_idx, i.previous_output_idx, i.previous_tx_hash, i.amount, i.address, i.coinbase,"
                        "i.sequence "
                "FROM bitcoin_transaction_inputs AS ti "
                "JOIN bitcoin_inputs AS i ON ti.input_uid = i.uid "
                "WHERE ti.transaction_hash = :hash ORDER BY ti.input_idx", use(out.hash)
            );
            for (auto& inputRow : inputRows) {
                BitcoinLikeBlockchainExplorerInput input;
                input.index = get_number<uint64_t>(inputRow, 0);
                input.previousTxOutputIndex = inputRow.get<Option<int>>(1).map<uint32_t>([] (const int& v) {
                    return (uint32_t) v;
                });
                input.previousTxHash = inputRow.get<Option<std::string>>(2);
                input.value = inputRow.get<Option<long long>>(3).map<BigInt>([] (const unsigned long long& v) {
                    return BigInt(v);
                });
                input.address = inputRow.get<Option<std::string>>(4);
                input.coinbase = inputRow.get<Option<std::string>>(5);
                input.sequence = get_number<uint32_t>(inputRow, 6);
                out.inputs.push_back(input);
            }

            // Fetch outputs
            //Check if the output belongs to this account
            //solve case of 2 accounts in DB one as sender and one as receiver
            //of same transaction (filter on account_uid won't solve the issue because
            //bitcoin_outputs going to external accounts have NULL account_uid)
            auto btcTxUid = BitcoinLikeTransactionDatabaseHelper::createBitcoinTransactionUid(accountUid, out.hash);
            rowset<soci::row> outputRows = (sql.prepare <<
                    "SELECT idx, amount, script, address, block_height FROM bitcoin_outputs WHERE transaction_hash = :hash AND transaction_uid = :tx_uid "
                    "ORDER BY idx", use(out.hash), use(btcTxUid)
            );

            for (auto& outputRow : outputRows) {
                BitcoinLikeBlockchainExplorerOutput output;
                output.index = (uint64_t) outputRow.get<int>(0);
                output.value.assignScalar(outputRow.get<long long>(1));
                output.script = outputRow.get<std::string>(2);
                output.address = outputRow.get<Option<std::string>>(3);
                if (outputRow.get_indicator(4) != i_null) {
                    output.blockHeight = row.get<BigInt>(5).toUint64();
                }
                out.outputs.push_back(std::move(output));
            }

            // Enjoy the silence.
            return true;
        }

        void
        BitcoinLikeTransactionDatabaseHelper::getMempoolTransactions(soci::session &sql, const std::string &accountUid,
                                                                     std::vector<BitcoinLikeBlockchainExplorerTransaction> &out) {
            // Query all transaction
            rowset<row> txRows = (sql.prepare <<
                    "SELECT  tx.hash, tx.version, tx.time, tx.locktime, "
                    "block.hash, block.height, block.time, block.currency_name "
                    "FROM bitcoin_transactions AS tx "
                    "LEFT JOIN blocks AS block ON tx.block_uid = block.uid "
                    "WHERE tx.hash IN ("
                      "SELECT tx.hash FROM operations AS op "
                      "JOIN bitcoin_operations AS bop ON bop.uid = op.uid  "
                      "JOIN bitcoin_transactions AS tx ON tx.hash = bop.transaction_hash "
                      "WHERE op.account_uid = :uid AND op.block_uid IS NULL"
                    ")", use(accountUid));
            // Inflate all transactions
            for (const auto& txRow : txRows) {
                BitcoinLikeBlockchainExplorerTransaction tx;
                inflateTransaction(sql, txRow, accountUid, tx);
                out.emplace_back(std::move(tx));
            }
        }

        void BitcoinLikeTransactionDatabaseHelper::removeAllMempoolOperation(soci::session &sql,
                                                                             const std::string &accountUid) {
            rowset<std::string> rows = (sql.prepare <<
                    "SELECT transaction_uid FROM bitcoin_operations AS bop "
                    "JOIN operations AS op ON bop.uid = op.uid "
                    "WHERE op.account_uid = :uid AND op.block_uid IS NULL", use(accountUid)
            );
            std::vector<std::string> txToDelete(rows.begin(), rows.end());
            if (!txToDelete.empty()) {
                sql << "DELETE FROM bitcoin_inputs WHERE uid IN ("
                       "SELECT input_uid FROM bitcoin_transaction_inputs "
                       "WHERE transaction_uid IN(:uids)"
                       ")", use(txToDelete);
                sql << "DELETE FROM operations WHERE account_uid = :uid AND block_uid is NULL", use(accountUid);
                sql << "DELETE FROM bitcoin_transactions "
                       "WHERE transaction_uid IN (:uids)", use(txToDelete);
            }
        }

    }
}
