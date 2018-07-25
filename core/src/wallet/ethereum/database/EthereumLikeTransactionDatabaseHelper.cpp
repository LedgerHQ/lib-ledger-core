/*
 *
 * EthereumLikeTransactionDatabaseHelper
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#include "EthereumLikeTransactionDatabaseHelper.h"
#include <math/BigInt.h>
#include <utils/hex.h>
#include <database/soci-option.h>
#include <database/soci-date.h>
#include <database/soci-number.h>

using namespace soci;

namespace ledger {
    namespace core {

        bool EthereumLikeTransactionDatabaseHelper::getTransactionByHash(soci::session &sql,
                                                                         const std::string &hash,
                                                                         EthereumLikeBlockchainExplorer::Transaction &tx) {

            rowset<row> rows = (sql.prepare << "SELECT  tx.hash, tx.nonce, tx.time, tx.input_data, tx.gas_price, "
                                                "tx.gas_limit, tx.gas_used, tx.sender, tx.receiver, tx.confirmations, "
                                                "block.hash, block.height, block.time, block.currency_name "
                                                "FROM ethereum_transactions AS tx "
                                                "LEFT JOIN blocks AS block ON tx.block_uid = block.uid "
                                                "WHERE tx.hash = :hash", use(hash));

            for (auto& row : rows) {
                inflateTransaction(sql, row, tx);
                return true;
            }

            return false;
        }

        bool EthereumLikeTransactionDatabaseHelper::inflateTransaction(soci::session &sql,
                                                                       const soci::row &row,
                                                                       EthereumLikeBlockchainExplorer::Transaction &tx) {
            tx.hash = row.get<std::string>(0);

            auto nonceBytes = hex::toByteArray(row.get<std::string>(1));
            auto shift = 0;
            for (auto& byte : nonceBytes) {
                tx.nonce += byte << shift;
                shift += 8;
            }

            tx.receivedAt = row.get<std::chrono::system_clock::time_point>(2);
            tx.inputData = hex::toByteArray(row.get<std::string>(3));

            tx.gasPrice = BigInt(row.get<std::string>(4));
            tx.gasLimit = BigInt(row.get<std::string>(5));
            tx.gasUsed = BigInt(row.get<std::string>(6));

            tx.receiver = row.get<std::string>(7);
            tx.sender = row.get<std::string>(8);

            tx.confirmations = get_number<uint64_t>(row, 9);

            if (row.get_indicator(10) != i_null) {
                EthereumLikeBlockchainExplorer::Block block;
                block.hash = row.get<std::string>(10);
                block.height = get_number<uint64_t>(row, 11);
                block.time = row.get<std::chrono::system_clock::time_point>(12);
                block.currencyName = row.get<std::string>(13);
                tx.block = block;
            }

            // Enjoy the silence.
            return true;
        }
    }
}
