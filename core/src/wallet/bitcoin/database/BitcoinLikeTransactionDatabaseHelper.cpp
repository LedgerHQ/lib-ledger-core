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
#include "BitcoinLikeBlockDatabaseHelper.h"
#include <database/soci-option.h>
#include <database/soci-date.h>
#include <database/soci-number.h>

using namespace soci;

namespace ledger {
    namespace core {

        bool BitcoinLikeTransactionDatabaseHelper::transactionExists(soci::session &sql, const std::string &transactionHash) {
            int32_t count = 0;
            sql << "SELECT COUNT(*) FROM bitcoin_transactions WHERE hash = :hash", use(transactionHash), into(count);
            return count == 1;
        }

        bool BitcoinLikeTransactionDatabaseHelper::putTransaction(soci::session &sql,
                                                                  const BitcoinLikeBlockchainExplorer::Transaction &tx) {
            auto blockHash = tx.block.map<std::string>([] (const BitcoinLikeBlockchainExplorer::Block& block) {
                                   return block.hash;
                               });
            if (transactionExists(sql, tx.hash)) {
                // UPDATE (we only update block information)

                return false;
            } else {
                // Insert
                if (tx.block.nonEmpty()) {
                    BitcoinLikeBlockDatabaseHelper::putBlock(sql, tx.block.getValue());
                }
                sql << "INSERT INTO bitcoin_transactions VALUES("
                        ":hash, :version, :block_hash, :time, :locktime"
                        ")",
                        use(tx.hash),
                        use(tx.version),
                        use(blockHash),
                        use(tx.receivedAt),
                        use(tx.lockTime);
                // Insert outputs
                for (const auto& output : tx.outputs) {
                    insertOutput(sql, tx.hash, output);
                }
                // Insert inputs
                for (const auto& input : tx.inputs) {
                    insertInput(sql, tx.hash, input);
                }
                return true;
            }
        }

        void BitcoinLikeTransactionDatabaseHelper::insertOutput(soci::session &sql,
                                                                const std::string& transactionHash,
                                                                const BitcoinLikeBlockchainExplorer::Output &output) {
            sql << "INSERT INTO bitcoin_outputs VALUES(:idx, :hash, :amount, :script, :address)",
                    use(output.index), use(transactionHash), use(output.value.toUint64()), use(output.script),
                    use(output.address);
        }

        void BitcoinLikeTransactionDatabaseHelper::insertInput(soci::session &sql,
                                                               const std::string& transactionHash,
                                                               const BitcoinLikeBlockchainExplorer::Input &input) {
            auto uid = createInputUid(input.previousTxOutputIndex.getValueOr(0),
                                      input.previousTxHash.getValueOr(""),
                                      input.coinbase.getValueOr("")
            );
            auto amount = input.value.map<uint64_t>([] (const BigInt& v) {
                return v.toUint64();
            });
            sql << "INSERT INTO bitcoin_inputs VALUES(:uid, :idx, :hash, :amount, :address, :coinbase, :sequence)",
                    use(uid), use(input.previousTxOutputIndex), use(input.previousTxHash), use(amount),
                    use(input.address), use(input.coinbase), use(input.sequence);
            sql << "INSERT INTO bitcoin_transaction_inputs VALUES(:tx, :input, :idx)",
                    use(transactionHash), use(uid), use(input.index);
        }

        std::string BitcoinLikeTransactionDatabaseHelper::createInputUid(int32_t previousOutputIndex,
                                                                         const std::string &previousTxHash,
                                                                         const std::string &coinbase) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}+{}", previousOutputIndex, previousTxHash, coinbase));
        }

        bool BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(soci::session &sql, const std::string &hash,
                                                                        BitcoinLikeBlockchainExplorer::Transaction &out) {
            rowset<row> rows = (sql.prepare <<
                    "SELECT  tx.hash, tx.version, tx.block_hash, "
                            "tx.time, tx.locktime, block.height, block.time "
                            "FROM bitcoin_transactions AS tx "
                            "LEFT JOIN bitcoin_blocks AS block ON tx.block_hash = block.hash "
                            "WHERE tx.hash = :hash", use(hash)
            );
            for (auto& row : rows) {
                inflateTransaction(sql, row, out);
                return true;
            }
            return false;
        }

        bool BitcoinLikeTransactionDatabaseHelper::inflateTransaction(soci::session &sql, const soci::row &row,
                                                                      BitcoinLikeBlockchainExplorer::Transaction &out) {
            out.hash = row.get<std::string>(0);
            out.version = (uint32_t) row.get<int32_t>(1);
            out.receivedAt = row.get<std::chrono::system_clock::time_point>(3);
            out.lockTime = (uint64_t) row.get<int>(4);
            if (row.get_indicator(2) != i_null) {
                BitcoinLikeBlockchainExplorer::Block block;
                block.hash = row.get<std::string>(2);
                block.height = (uint64_t) row.get<long long>(5);
                block.time = row.get<std::chrono::system_clock::time_point>(6);
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
                BitcoinLikeBlockchainExplorer::Input input;
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
            rowset<soci::row> outputRows = (sql.prepare <<
                    "SELECT idx, amount, script, address FROM bitcoin_outputs WHERE transaction_hash = :hash "
                    "ORDER BY idx", use(out.hash)
            );
            for (auto& outputRow : outputRows) {
                BitcoinLikeBlockchainExplorer::Output output;
                output.index = (uint64_t) outputRow.get<int>(0);
                output.value.assignScalar(outputRow.get<long long>(1));
                output.script = outputRow.get<std::string>(2);
                output.address = outputRow.get<Option<std::string>>(3);
                out.outputs.push_back(std::move(output));
            }

            // Enjoy the silence.
            return true;
        }

    }
}
