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
#include "BitcoinLikeWalletDatabase.h"
#include "../../pool/database/WalletDatabaseEntry.hpp"
#include <soci.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/bitcoin/database/BitcoinLikeAccountDatabaseHelper.h>

using namespace soci;

namespace ledger {
    namespace core {

        BitcoinLikeWalletDatabase::BitcoinLikeWalletDatabase(const std::shared_ptr<WalletPool> &pool,
                                                     const std::string &walletName,
                                                     const std::string& currencyName)
                : _walletUid(WalletDatabaseEntry::createWalletUid(pool->getName(), walletName)) {
            _database = pool->getDatabaseSessionPool();
        }

        int64_t BitcoinLikeWalletDatabase::getAccountsCount() const {
            session sql(_database->getPool());
            int64_t count = 0L;
            sql << "SELECT COUNT(*) FROM bitcoin_accounts WHERE wallet_uid = :uid", use(_walletUid), into(count);
            return count;
        }

        bool BitcoinLikeWalletDatabase::accountExists(int32_t index) const {
            session sql(_database->getPool());
            int64_t count = 0L;
            auto uid = AccountDatabaseHelper::createAccountUid(_walletUid, index);
            sql << "SELECT COUNT(*) FROM bitcoin_accounts WHERE uid = :uid", use(uid), into(count);
            return count == 1;
        }

        void BitcoinLikeWalletDatabase::createAccount(int32_t index, const std::string &xpub) const {
            session sql(_database->getPool());
            BitcoinLikeAccountDatabaseHelper::createAccount(sql, getWalletUid(), index, xpub);
        }

        int32_t BitcoinLikeWalletDatabase::getNextAccountIndex() const {
            session sql(_database->getPool());
            return AccountDatabaseHelper::computeNextAccountIndex(sql, _walletUid);
        }

        const std::string &BitcoinLikeWalletDatabase::getWalletUid() const {
            return _walletUid;
        }

    }
}