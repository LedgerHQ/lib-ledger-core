/*
 *
 * BlockDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/07/2017.
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
#include "BlockDatabaseHelper.h"
#include <crypto/SHA256.hpp>
#include <fmt/format.h>
#include <database/soci-date.h>
#include <database/soci-number.h>

using namespace soci;

namespace ledger {
    namespace core {

        bool BlockDatabaseHelper::putBlock(soci::session &sql, const Block &block) {
            if (!blockExists(sql, block.hash, block.currencyName)) {
                auto uid = createBlockUid(block);
                sql << "INSERT INTO blocks VALUES(:uid, :hash, :height, :time, :currency_name)",
                        use(uid), use(block.hash), use(block.height), use(block.time), use(block.currencyName);
                return true;
            }
            return false;
        }


        std::string BlockDatabaseHelper::createBlockUid(const Block &block) {
            return createBlockUid(block.hash, block.currencyName);
        }

        bool BlockDatabaseHelper::blockExists(soci::session &sql, const std::string &blockHash,
                                              const std::string &currencyName) {
            auto count = 0;
            auto uid = createBlockUid(blockHash, currencyName);
            sql << "SELECT COUNT(*) FROM blocks WHERE uid = :uid", use(uid), into(count);
            return count > 0;
        }

        std::string BlockDatabaseHelper::createBlockUid(const std::string &blockhash, const std::string &currencyName) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}", blockhash, currencyName));
        }

        Option<api::Block> BlockDatabaseHelper::getLastBlock(soci::session &sql, const std::string &currencyName) {
            rowset<row> rows = (sql.prepare << "SELECT uid, hash, height, time FROM blocks WHERE "
                    "currency_name = :name ORDER BY height DESC LIMIT 1", use(currencyName));
            for (auto& row : rows) {
                auto uid = row.get<std::string>(0);
                auto hash = row.get<std::string>(1);
                auto height = get_number<int64_t>(row, 2);
                auto time = row.get<std::chrono::system_clock::time_point>(3);
                return Option<api::Block>(
                        api::Block(hash, uid, time, currencyName, height)
                );
            }
            return Option<api::Block>();
        }
    }
}