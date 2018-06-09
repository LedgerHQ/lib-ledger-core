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
#ifndef LEDGER_CORE_BLOCKDATABASEHELPER_H
#define LEDGER_CORE_BLOCKDATABASEHELPER_H

#include <wallet/common/Block.h>
#include <string>
#include <soci.h>
#include <api/Block.hpp>
#include <utils/Option.hpp>

namespace ledger {
    namespace core {
        class BlockDatabaseHelper {
        public:
            static std::string createBlockUid(const Block& block);
            static std::string createBlockUid(const std::string& blockhash, const std::string& currencyName);
            static bool putBlock(soci::session& sql, const Block& block);
            static bool blockExists(soci::session& sql, const std::string& blockHash, const std::string& currencyName);
            static Option<api::Block> getLastBlock(soci::session& sql, const std::string& currencyName);
            static Option<api::Block> getPreviousBlockInDatabase(soci::session& sql, const std::string& currencyName, int64_t blockHeight);
            static Option<api::Block> getPreviousBlockInDatabase(soci::session &sql, const std::string &currencyName, std::chrono::system_clock::time_point date);
        };
    }
}


#endif //LEDGER_CORE_BLOCKDATABASEHELPER_H
