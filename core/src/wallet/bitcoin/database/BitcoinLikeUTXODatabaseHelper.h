/*
 *
 * BitcoinLikeUTXODatabaseHelper.h
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

#ifndef LEDGER_CORE_BITCOINLIKEUTXODATABASEHELPER_H
#define LEDGER_CORE_BITCOINLIKEUTXODATABASEHELPER_H

#include <soci.h>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeUtxo.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeUTXODatabaseHelper {
            BitcoinLikeUTXODatabaseHelper()  = delete;

            ~BitcoinLikeUTXODatabaseHelper() = delete;

          public:
            static std::size_t queryUTXO(soci::session &sql,
                                         const std::string &accountUid,
                                         int32_t offset,
                                         int32_t count,
                                         int64_t dustAmount,
                                         std::vector<BitcoinLikeBlockchainExplorerOutput> &out,
                                         const std::function<bool(const std::string &address)> &filter);

            static std::size_t UTXOcount(soci::session &sql,
                                         const std::string &accountUid,
                                         int64_t dustAmount,
                                         const std::function<bool(const std::string &address)> &filter);

            static std::vector<BitcoinLikeUtxo> queryAllUtxos(
                soci::session &session,
                std::string const &accountUid,
                api::Currency const &currency,
                int64_t dustAmount);
        };
    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_BITCOINLIKEUTXODATABASEHELPER_H
