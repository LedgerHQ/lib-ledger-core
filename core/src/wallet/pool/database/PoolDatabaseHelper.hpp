/*
 *
 * PoolDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 16/05/2017.
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
#ifndef LEDGER_CORE_POOLDATABASEHELPER_HPP
#define LEDGER_CORE_POOLDATABASEHELPER_HPP

#include <soci.h>
#include <wallet/pool/WalletPool.hpp>
#include "WalletDatabaseEntry.hpp"

namespace ledger {
    namespace core {
        class PoolDatabaseHelper {
        public:
            static bool insertPool(soci::session& sql, const WalletPool& pool);
            static void putWallet(soci::session &sql, const WalletDatabaseEntry &wallet);
            static int64_t getWallets(soci::session& sql,
                                      const WalletPool& pool, int64_t offset,
                                      std::vector<WalletDatabaseEntry>& wallets);
            static int64_t getWalletCount(soci::session& sql, const WalletPool& pool);
            static bool getWallet(soci::session& sql, const WalletPool& pool, const std::string& walletName, WalletDatabaseEntry& entry);
            static bool removeWallet(soci::session& sql, const WalletDatabaseEntry& entry);
            static void removeWalletByName(soci::session& sql, const std::string& name);
            static bool walletExists(soci::session& sql, const WalletDatabaseEntry& entry);

        private:
            PoolDatabaseHelper() = delete;
            static void inflateWalletEntry(soci::row& row, const WalletPool& pool, WalletDatabaseEntry& entry);
        };
    }
}


#endif //LEDGER_CORE_POOLDATABASEHELPER_HPP
