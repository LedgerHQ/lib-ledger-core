/*
 *
 * BitcoinLikeUTXODatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/09/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "BitcoinLikeUTXODatabaseHelper.h"

#include <database/soci-number.h>
#include <database/soci-option.h>
#include <utils/Option.hpp>
#include <wallet/bitcoin/api_impl/BitcoinLikeOutputApi.h>

using namespace soci;

namespace ledger {
    namespace core {

        std::size_t BitcoinLikeUTXODatabaseHelper::UTXOcount(soci::session &sql, const std::string &accountUid, int64_t dustAmount) {
            const rowset<row> rows = (sql.prepare << "SELECT o.address FROM bitcoin_outputs AS o "
                                                     " LEFT OUTER JOIN bitcoin_inputs AS i ON i.previous_tx_uid = o.transaction_uid "
                                                     " AND i.previous_output_idx = o.idx"
                                                     " WHERE i.previous_tx_uid IS NULL AND o.account_uid = :uid AND o.amount > :dustAmount",
                                      use(accountUid), use(dustAmount));
            std::size_t count      = 0;
            for (auto &row : rows) {
                if (row.get_indicator(0) != i_null) {
                    count += 1;
                }
            }
            return count;
        }

        std::size_t
        BitcoinLikeUTXODatabaseHelper::queryUTXO(soci::session &sql, const std::string &accountUid, int32_t offset, int32_t count, int64_t dustAmount, std::vector<BitcoinLikeBlockchainExplorerOutput> &out) {
            const rowset<row> rows = (sql.prepare << "SELECT o.address, o.idx, o.transaction_hash, o.amount, o.script, o.block_height,"
                                                     "replaceable"
                                                     " FROM bitcoin_outputs AS o "
                                                     " LEFT OUTER JOIN bitcoin_inputs AS i ON i.previous_tx_uid = o.transaction_uid "
                                                     " AND i.previous_output_idx = o.idx"
                                                     " WHERE i.previous_tx_uid IS NULL AND o.account_uid = :uid AND o.amount > :dustAmount"
                                                     " ORDER BY block_height LIMIT :count OFFSET :off",
                                      use(accountUid), use(dustAmount), use(count), use(offset));

            for (auto &row : rows) {
                if (row.get_indicator(0) != i_null) {
                    BitcoinLikeBlockchainExplorerOutput output;

                    output.address         = row.get<Option<std::string>>(0);
                    output.index           = get_number<uint64_t>(row, 1);
                    output.transactionHash = row.get<std::string>(2);
                    output.value           = row.get<BigInt>(3);
                    output.script          = row.get<std::string>(4);
                    if (row.get_indicator(5) != i_null) {
                        output.blockHeight = soci::get_number<uint64_t>(row, 5);
                    }
                    output.replaceable = soci::get_number<int>(row, 6) == 1;
                    out.emplace_back(output);
                }
            }
            return out.size();
        }

        constexpr auto uncachedBalanceQuery = R"(
                SELECT sum(o.amount)::bigint
                FROM bitcoin_outputs AS o
                    LEFT OUTER JOIN bitcoin_inputs AS i ON i.previous_tx_uid = o.transaction_uid
                    AND i.previous_output_idx = o.idx
                WHERE i.previous_tx_uid IS NULL
                    AND o.account_uid = :uid)";

        BigInt getUncachedBalance(soci::session &sql, const std::string &accountUid) {
            const rowset<soci::row> rows = (sql.prepare << uncachedBalanceQuery, use(accountUid));
            for (auto &row : rows) {
                if (row.get_indicator(0) != i_null) {
                    return row.get<BigInt>(0);
                }
            }
            return BigInt(0);
        }

        BigInt BitcoinLikeUTXODatabaseHelper::getBalance(soci::session &sql, const std::string &accountUid) {
            const rowset<row> rows = (sql.prepare << "SELECT balance from bitcoin_accounts WHERE uid=:uid",
                                      use(accountUid));

            for (auto &row : rows) {
                if (row.get_indicator(0) != i_null) {
                    auto balance = row.get<BigInt>(0);
                    if (balance.isNegative()) {
                        // We compute balance from DB instead of updating it, to let only the sync (lama-bitcoin or libcore, depending on the coin) doing the update
                        return getUncachedBalance(sql, accountUid);
                    }
                    return balance;
                }
            }
            return BigInt(0);
        }

        void BitcoinLikeUTXODatabaseHelper::updateBalance(session &sql, const std::string &accountUid) {
            const rowset<row> rows = (sql.prepare << std::string{"UPDATE bitcoin_accounts SET balance = ("} + uncachedBalanceQuery + ") WHERE uid = :uid;",
                                      use(accountUid), use(accountUid));
        }

        std::vector<BitcoinLikeUtxo> BitcoinLikeUTXODatabaseHelper::queryAllUtxos(
            soci::session &session,
            std::string const &accountUid,
            api::Currency const &currency,
            int64_t dustAmount) {
            const soci::rowset<soci::row> rows = (session.prepare << "SELECT o.address, o.idx, o.transaction_hash, o.amount, o.script, o.block_height "
                                                                     "FROM bitcoin_outputs AS o "
                                                                     "LEFT OUTER JOIN bitcoin_inputs AS i ON i.previous_tx_uid = o.transaction_uid AND i.previous_output_idx = o.idx "
                                                                     "WHERE i.previous_tx_uid IS NULL AND o.account_uid = :uid AND o.amount > :dustAmount "
                                                                     "ORDER BY o.block_height",
                                                  use(accountUid), use(dustAmount));

            std::vector<BitcoinLikeUtxo> utxos;

            for (auto &row : rows) {
                if (row.get_indicator(0) != i_null) {
                    const BitcoinLikeUtxo output{
                        get_number<uint64_t>(row, 1),
                        row.get<std::string>(2),
                        Amount(currency, 0, row.get<BigInt>(3)),
                        row.get<Option<std::string>>(0),
                        accountUid,
                        row.get<std::string>(4),
                        row.get_indicator(5) != i_null ? Option<uint64_t>{row.get<BigInt>(5).toUint64()} : Option<uint64_t>{}};

                    utxos.push_back(output);
                }
            }

            return utxos;
        }
    } // namespace core
} // namespace ledger
