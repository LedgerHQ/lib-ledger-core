/*
 *
 * StellarLikeLedgerDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/07/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include "StellarLikeLedgerDatabaseHelper.hpp"
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <database/soci-date.h>
#include <database/soci-number.h>

using namespace soci;

namespace ledger {
    namespace core {

        bool StellarLikeLedgerDatabaseHelper::putLedger(soci::session &sql, const api::Currency& currency, const stellar::Ledger &ledger) {
            Block block;
            block.hash = ledger.hash;
            block.height = ledger.height;
            block.time = ledger.time;
            block.currencyName = currency.name;
            if (BlockDatabaseHelper::putBlock(sql, block)) {
                auto uid = BlockDatabaseHelper::createBlockUid(ledger.hash, currency.name);
                auto baseFee = ledger.baseFee.toString();
                auto baseReserve = ledger.baseReserve.toString();
                sql << "INSERT INTO stellar_ledgers VALUES(:uid, :base_fee, :base_reserve)",
                use(uid), use(baseFee), use(baseReserve);
                return true;
            }
            return false;
        }

        bool StellarLikeLedgerDatabaseHelper::getLastLedger(soci::session &sql, const api::Currency &currency,
                                                            stellar::Ledger &out) {
            rowset<row> rows = (sql.prepare <<
                    "SELECT b.hash, b.height, b.time, l.base_fee, l.base_reserve "
                    "FROM stellar_ledgers AS l "
                    "LEFT JOIN blocks AS b ON l.uid = b.uid "
                    "WHERE b.currency_name = :name "
                    "ORDER BY b.height DESC LIMIT 1", use(currency.name));
            for (const auto& row : rows) {
                out.hash = row.get<std::string>(0);
                out.height = soci::get_number<uint64_t>(row, 1);
                out.time = row.get<std::chrono::system_clock::time_point>(2);
                out.baseFee = BigInt::fromString(row.get<std::string>(3));
                out.baseReserve = BigInt::fromString(row.get<std::string>(4));
                return true;
            }
            return false;
        }

    }
}