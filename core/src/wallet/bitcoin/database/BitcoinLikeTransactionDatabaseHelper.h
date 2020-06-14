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
#ifndef LEDGER_CORE_BITCOINLIKETRANSACTIONDATABASEHELPER_H
#define LEDGER_CORE_BITCOINLIKETRANSACTIONDATABASEHELPER_H

#include <soci.h>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeTransactionDatabaseHelper {
        public:
            static bool transactionExists(soci::session& sql, const std::string& btcTxUid);
            static std::string putTransaction(soci::session& sql,
                                              const std::string &accountUid,
                                              const BitcoinLikeBlockchainExplorerTransaction& tx);
            static inline void insertOutput(soci::session& sql,
                                            const std::string& btcTxUid,
                                            const std::string &accountUid,
                                            const std::string& transactionHash,
                                            const BitcoinLikeBlockchainExplorerOutput& output);
            static inline void insertInput(soci::session& sql,
                                           const std::string& btcTxUid,
                                           const std::string& accountUid,
                                           const std::string& transactionHash,
                                           const BitcoinLikeBlockchainExplorerInput& input);

            static std::string createInputUid(const std::string& accountUid, int32_t previousOutputIndex, const std::string& previousTxHash, const std::string& coinbase);
            static std::string createBitcoinTransactionUid(const std::string& accountUid, const std::string& txHash);
            static bool getTransactionByHash(soci::session &sql,
                                             const std::string &hash,
                                             const std::string &accountUid,
                                             BitcoinLikeBlockchainExplorerTransaction &out);

            static inline bool inflateTransaction(soci::session& sql,
                                                  const soci::row& row,
                                                  const std::string &accountUid,
                                                  BitcoinLikeBlockchainExplorerTransaction& out);

            static void getMempoolTransactions(soci::session& sql,
                                               const std::string& accountUid,
                                               std::vector<BitcoinLikeBlockchainExplorerTransaction>& out);
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKETRANSACTIONDATABASEHELPER_H
