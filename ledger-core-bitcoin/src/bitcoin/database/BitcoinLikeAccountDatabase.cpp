/*
 *
 * BitcoinLikeAccountDatabase
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
#include <core/database/SociOption.hpp>
#include <core/database/SociDate.hpp>
#include <core/utils/DateUtils.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>

#include <bitcoin/database/BitcoinLikeAccountDatabase.hpp>

using namespace soci;

namespace ledger {
    namespace core {

        BitcoinLikeAccountDatabase::BitcoinLikeAccountDatabase(const std::string &walletUid, int32_t index) {
            _accountUid = AccountDatabaseHelper::createAccountUid(walletUid, index);
        }


    }
}
