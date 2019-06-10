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

#include <fmt/format.h>

#include <core/crypto/SHA256.hpp>
#include <core/database/soci-date.h>
#include <core/database/soci-number.h>
#include <core/wallet/BlockDatabaseHelper.h>

using namespace soci;

namespace ledger {
    namespace core {
        bool BlockDatabaseHelper::putBlock(soci::session &sql, const api::Block &block) {
            if (!blockExists(sql, block.blockHash, block.currencyName)) {
                auto uid = createBlockUid(block);
                sql << "INSERT INTO blocks VALUES(:uid, :hash, :height, :time, :currency_name)",
                        use(uid), use(block.blockHash), use(block.height), use(block.time), use(block.currencyName);
                return true;
            }
            return false;
        }


        std::string BlockDatabaseHelper::createBlockUid(const api::Block &block) {
            return createBlockUid(block.blockHash, block.currencyName);
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

        static Option<api::Block> getBlockFromRow(row &databaseRow, const std::string &currencyName) {
            auto uid = databaseRow.get<std::string>(0);
            auto hash = databaseRow.get<std::string>(1);
            auto height = get_number<int64_t>(databaseRow, 2);
            auto time = databaseRow.get<std::chrono::system_clock::time_point>(3);
            return Option<api::Block>(
                    api::Block(hash, uid, time, currencyName, height)
            );
        }
        Option<api::Block> BlockDatabaseHelper::getLastBlock(soci::session &sql, const std::string &currencyName) {
            rowset<row> rows = (sql.prepare << "SELECT uid, hash, height, time FROM blocks WHERE "
                    "currency_name = :name ORDER BY height DESC LIMIT 1", use(currencyName));
            for (auto& row : rows) {
                return getBlockFromRow(row, currencyName);
            }
            return Option<api::Block>();
        }

        Option<api::Block> BlockDatabaseHelper::getPreviousBlockInDatabase(soci::session &sql, const std::string &currencyName, int64_t blockHeight) {
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT uid, hash, height, time FROM blocks WHERE "
                    "currency_name = :name AND height < :blockHeight ORDER BY height DESC LIMIT 1",
                    soci::use(currencyName), soci::use(blockHeight));

            for (auto& row : rows) {
                return getBlockFromRow(row, currencyName);
            }
            return Option<api::Block>();
        }

        Option<api::Block> BlockDatabaseHelper::getPreviousBlockInDatabase(soci::session &sql, const std::string &currencyName, std::chrono::system_clock::time_point date) {
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT uid, hash, height, time FROM blocks WHERE "
                    "currency_name = :name AND time < :date ORDER BY height DESC LIMIT 1",
                    soci::use(currencyName), soci::use(date));

            for (auto& row : rows) {
                return getBlockFromRow(row, currencyName);
            }
            return Option<api::Block>();
        }
    }
}
