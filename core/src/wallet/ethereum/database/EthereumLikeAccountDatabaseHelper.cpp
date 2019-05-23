/*
 *
 * EthereumLikeAccountDatabaseHelper
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


#include "EthereumLikeAccountDatabaseHelper.h"
#include <wallet/common/database/AccountDatabaseHelper.h>

using namespace soci;

namespace ledger {
    namespace core {
        void EthereumLikeAccountDatabaseHelper::createAccount(soci::session &sql,
                                                         const std::string walletUid, int32_t index,
                                                         const std::string &address) {
            auto uid = AccountDatabaseHelper::createAccountUid(walletUid, index);
            sql << "INSERT INTO ethereum_accounts VALUES(:uid, :wallet_uid, :idx, :address)",use(uid), use(walletUid), use(index), use(address);
        }

        void EthereumLikeAccountDatabaseHelper::createERC20Account(soci::session &sql,
                                                                   const std::string &ethAccountUid,
                                                                   const std::string &erc20AccountUid,
                                                                   const std::string &contractAddress) {
            sql << "INSERT INTO erc20_accounts VALUES(:uid, :ethereum_account_uid, :contract_address)",use(erc20AccountUid), use(ethAccountUid), use(contractAddress);
        }

        bool EthereumLikeAccountDatabaseHelper::queryAccount(soci::session &sql,
                                                             const std::string &accountUid,
                                                             EthereumLikeAccountDatabaseEntry &entry) {
            rowset<row> rows = (sql.prepare << "SELECT eth.idx, eth.address, "
                    "erc20.uid, erc20.contract_address "
                    "FROM ethereum_accounts AS eth "
                    "LEFT JOIN erc20_accounts AS erc20 ON erc20.ethereum_account_uid = eth.uid"
                    " WHERE eth.uid = :uid", use(accountUid));
            for (auto& row : rows) {
                if (entry.address.empty()) {
                    entry.index = row.get<int32_t>(0);
                    entry.address = row.get<std::string>(1);
                }
                if (row.get_indicator(2) != i_null) {
                    ERC20LikeAccountDatabaseEntry erc20Entry;
                    erc20Entry.uid = row.get<std::string>(2);
                    erc20Entry.contractAddress = row.get<std::string>(3);
                    entry.erc20Accounts.emplace_back(erc20Entry);
                }
            }
            return !entry.address.empty();
        }

        api::ERC20Token EthereumLikeAccountDatabaseHelper::getOrCreateERC20Token(soci::session &sql,
                                                                                 const std::string &contractAddress) {
            api::ERC20Token erc20Token;
            auto count = 0;
            sql << "SELECT COUNT(*) FROM erc20_tokens WHERE contract_address = :contract_address", soci::use(contractAddress), soci::into(count);
            if (count > 0) {
                soci::rowset<soci::row> rows = (sql.prepare << "SELECT name, symbol, number_of_decimal FROM erc20_tokens WHERE contract_address = :contract_address", soci::use(contractAddress));
                for (auto& row : rows) {
                    auto name = row.get<std::string>(0);
                    auto symbol = row.get<std::string>(1);
                    auto numberOfDecimals = row.get<int32_t>(2);
                    erc20Token = api::ERC20Token(name, symbol, contractAddress, numberOfDecimals);
                }
            } else {
                erc20Token = api::ERC20Token("UNKNOWN_TOKEN", "UNKNOWN", contractAddress, 0);
                sql << "INSERT INTO erc20_tokens VALUES(:contract_address, :name, :symbol, :number_of_decimal)",
                        use(erc20Token.contractAddress),
                        use(erc20Token.name),
                        use(erc20Token.symbol),
                        use(erc20Token.numberOfDecimal);
            }
            return erc20Token;
        }

    }
}