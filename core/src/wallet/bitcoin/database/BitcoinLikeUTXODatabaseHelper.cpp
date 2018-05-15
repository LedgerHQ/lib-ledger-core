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

#include <wallet/bitcoin/api_impl/BitcoinLikeOutputApi.h>
#include "BitcoinLikeUTXODatabaseHelper.h"
#include <database/soci-number.h>
#include <database/soci-option.h>
#include <iostream>
using namespace std;
using namespace soci;

namespace ledger {
    namespace core {

        std::size_t BitcoinLikeUTXODatabaseHelper::UTXOcount(soci::session &sql, const std::string &accountUid,
                                                             std::function<bool(const std::string &address)> filter) {
            rowset<row> rows = (sql.prepare <<
                                            "SELECT o.address FROM bitcoin_outputs AS o "
                                                    " LEFT OUTER JOIN bitcoin_inputs AS i ON i.previous_tx_hash = o.transaction_hash "
                                                    " AND i.previous_output_idx = o.idx"
                                                    " WHERE i.previous_tx_hash IS NULL AND o.account_uid = :uid", use(accountUid));
            std::size_t count = 0;
            for (auto& row : rows) {
                if (row.get_indicator(0) != i_null && filter(row.get<std::string>(0)))
                    count += 1;
            }
            return count;
        }

        std::size_t
        BitcoinLikeUTXODatabaseHelper::queryUTXO(soci::session &sql, const std::string &accountUid, int32_t offset,
                                                 int32_t count, std::vector<BitcoinLikeBlockchainExplorer::Output> &out,
                                                 std::function<bool(const std::string &address)> filter) {
            rowset<row> rows = (sql.prepare <<
                                            "SELECT o.address, o.idx, o.transaction_hash, o.amount, o.script"
                                                    " FROM bitcoin_outputs AS o "
                                                    " LEFT OUTER JOIN bitcoin_inputs AS i ON  i.previous_tx_hash = o.transaction_hash "
                                                    " AND i.previous_output_idx = o.idx"
                                                    " WHERE  i.previous_tx_hash IS NULL AND o.account_uid = :uid", use(accountUid));
            std::size_t c = 0;
            std::size_t o = 0;
            for (auto& row : rows) {
                if (row.get_indicator(0) != i_null && filter(row.get<std::string>(0))) {
                    if (o >= offset) {
                        out.resize(out.size() + 1);
                        auto& output = out[out.size() - 1];
                        output.address = row.get<Option<std::string>>(0);
                        output.index = get_number<uint64_t>(row, 1);
                        output.transactionHash = row.get<std::string>(2);
                        output.value = row.get<BigInt>(3);
                        output.script = row.get<std::string>(4);
                        c += 1;
                    }
                    o += 1;
                }
                if (c >= count)
                    break;
            }
            return c;
        }



    }
}