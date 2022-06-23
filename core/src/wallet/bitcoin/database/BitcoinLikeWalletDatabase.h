/*
 *
 * BitcoinLikeSociWallet
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/05/2017.
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
#ifndef LEDGER_CORE_BITCOINLIKESOCIWALLET_H
#define LEDGER_CORE_BITCOINLIKESOCIWALLET_H

#include "BitcoinLikeAccountDatabase.h"

#include <database/DatabaseSessionPool.hpp>
#include <soci.h>
#include <wallet/pool/WalletPool.hpp>

namespace ledger {
    namespace core {

        class BitcoinLikeWalletDatabase {
          public:
            BitcoinLikeWalletDatabase(const std::shared_ptr<WalletPool> &pool,
                                      const std::string &walletName,
                                      const std::string &currencyName);
            int64_t getAccountsCount() const;
            bool accountExists(int32_t index) const;
            void createAccount(int32_t index, const std::string &xpub) const;
            const std::string &getWalletUid() const;
            int32_t getNextAccountIndex() const;

          private:
            const std::string _walletUid;
            mutable std::shared_ptr<DatabaseSessionPool> _database;
        };
    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_BITCOINLIKESOCIWALLET_H
