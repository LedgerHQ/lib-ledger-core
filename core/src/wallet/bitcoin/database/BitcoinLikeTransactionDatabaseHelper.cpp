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
                "SELECT DISTINCT ON(ti.input_idx) ti.input_idx, i.previous_output_idx, i.previous_tx_hash, i.amount, i.address, i.coinbase,"
                        "i.sequence "
                "FROM bitcoin_transaction_inputs AS ti "
                "INNER JOIN bitcoin_inputs AS i ON ti.input_uid = i.uid "
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
                    "SELECT idx, amount, script, address, block_height, replaceable "
                    "FROM bitcoin_outputs WHERE transaction_hash = :hash AND transaction_uid = :tx_uid "
                    "ORDER BY idx", use(out.hash), use(btcTxUid)
            );

            for (auto& outputRow : outputRows) {
                BitcoinLikeBlockchainExplorerOutput output;
                output.index = (uint64_t) outputRow.get<int>(0);
                output.value.assignScalar(outputRow.get<long long>(1));
                output.script = outputRow.get<std::string>(2);
                output.address = outputRow.get<Option<std::string>>(3);
                if (outputRow.get_indicator(4) != i_null) {
                    output.blockHeight = soci::get_number<uint64_t>(outputRow, 4);
                }
                output.replaceable = soci::get_number<int>(outputRow, 5) == 1;
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

        void BitcoinLikeTransactionDatabaseHelper::eraseDataSince(
                soci::session &sql,
                const std::string &accountUid,
                const std::chrono::system_clock::time_point & date) {
            
            rowset<std::string> rows = (sql.prepare <<
                "SELECT transaction_uid FROM bitcoin_operations AS bop "
                "JOIN operations AS op ON bop.uid = op.uid "
                "WHERE op.account_uid = :uid AND op.date >= :date", 
                use(accountUid), use(date)
            );
            std::vector<std::string> txToDelete(rows.begin(), rows.end());
            if (!txToDelete.empty()) {
                sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date", 
                    use(accountUid), use(date);
                sql << "DELETE FROM bitcoin_inputs WHERE uid IN ("
                       "SELECT input_uid FROM bitcoin_transaction_inputs "
                       "WHERE transaction_uid IN(:uids)"
                       ")", use(txToDelete);
                sql << "DELETE FROM bitcoin_transactions "
                       "WHERE transaction_uid IN (:uids)", use(txToDelete);
            }
        }

    }
}
