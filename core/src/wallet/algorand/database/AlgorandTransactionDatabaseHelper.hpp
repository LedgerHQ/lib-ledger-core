/*
 * AlgorandTransactionDatabaseHelper
 *
 * Created by Hakim Aammar on 18/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#ifndef LEDGER_CORE_ALGORANDTRANSACTIONDATABASEHELPER_H
#define LEDGER_CORE_ALGORANDTRANSACTIONDATABASEHELPER_H

#include "../model/transactions/AlgorandTransaction.hpp"
#include <math/BaseConverter.hpp>
#include <boost-optional.h>
#include <database/soci-number.h>
#include <chrono>

namespace ledger {
namespace core {
namespace algorand {

    static constexpr auto COL_TX_HASH = 1;
    static constexpr auto COL_TX_TYPE = 2;
    static constexpr auto COL_TX_ROUND = 3;
    static constexpr auto COL_TX_TIMESTAMP = 4;
    static constexpr auto COL_TX_FIRST_VALID = 5;
    static constexpr auto COL_TX_LAST_VALID = 6;
    static constexpr auto COL_TX_GENESIS_ID = 7;
    static constexpr auto COL_TX_GENESIS_HASH = 8;
    static constexpr auto COL_TX_SENDER = 9;
    static constexpr auto COL_TX_FEE = 10;
    static constexpr auto COL_TX_NOTE = 11;
    static constexpr auto COL_TX_GROUP = 12;
    static constexpr auto COL_TX_LEASE = 13;
    static constexpr auto COL_TX_SENDER_REWARDS = 14;
    static constexpr auto COL_TX_RECEIVER_REWARDS = 15;
    static constexpr auto COL_TX_CLOSE_REWARDS = 16;

    static constexpr auto COL_TX_PAY_AMOUNT = 17;
    static constexpr auto COL_TX_PAY_RECEIVER_ADDRESS = 18;
    static constexpr auto COL_TX_PAY_CLOSE_ADDRESS = 19;
    static constexpr auto COL_TX_PAY_CLOSE_AMOUNT = 20;

    static constexpr auto COL_TX_KEYREG_NONPART = 21;
    static constexpr auto COL_TX_KEYREG_SELECTION_PK = 22;
    static constexpr auto COL_TX_KEYREG_VOTE_PK = 23;
    static constexpr auto COL_TX_KEYREG_VOTE_KEY_DILUTION = 24;
    static constexpr auto COL_TX_KEYREG_VOTE_FIRST = 25;
    static constexpr auto COL_TX_KEYREG_VOTE_LAST = 26;

    static constexpr auto COL_TX_ACFG_ASSET_ID = 27;
    static constexpr auto COL_TX_ACFG_ASSET_NAME = 28;
    static constexpr auto COL_TX_ACFG_UNIT_NAME = 29;
    static constexpr auto COL_TX_ACFG_TOTAL = 30;
    static constexpr auto COL_TX_ACFG_DECIMALS = 31;
    static constexpr auto COL_TX_ACFG_DEFAULT_FROZEN = 32;
    static constexpr auto COL_TX_ACFG_CREATOR_ADDRESS = 33;
    static constexpr auto COL_TX_ACFG_MANAGER_ADDRESS = 34;
    static constexpr auto COL_TX_ACFG_RESERVE_ADDRESS = 35;
    static constexpr auto COL_TX_ACFG_FREEZE_ADDRESS = 36;
    static constexpr auto COL_TX_ACFG_CLAWBACK_ADDRESS = 37;
    static constexpr auto COL_TX_ACFG_METADATA_HASH = 38;
    static constexpr auto COL_TX_ACFG_URL = 39;

    static constexpr auto COL_TX_AXFER_ASSET_ID = 40;
    static constexpr auto COL_TX_AXFER_ASSET_AMOUNT = 41;
    static constexpr auto COL_TX_AXFER_RECEIVER_ADDRESS = 42;
    static constexpr auto COL_TX_AXFER_CLOSE_ADDRESS = 43;
    static constexpr auto COL_TX_AXFER_CLOSE_AMOUNT = 44;
    static constexpr auto COL_TX_AXFER_SENDER_ADDRESS = 45;

    static constexpr auto COL_TX_AFRZ_ASSET_ID = 46;
    static constexpr auto COL_TX_AFRZ_FROZEN = 47;
    static constexpr auto COL_TX_AFRZ_FROZEN_ADDRESS = 48;


    // Helpers to deal with Option<> types,
    // because SOCI only understands boost::optional<>...

    template<class Raw>
    static auto optionalValue(Option<Raw> opt) {
        return opt.hasValue() ? *opt : boost::optional<Raw>();
    }

    template<class Raw, class Transformed>
    static auto optionalValueWithTransform(Option<Raw> opt, const std::function<Transformed(Raw)> & transform) {
        return opt.hasValue() ? transform(*opt) : boost::optional<Transformed>();
    }

    static std::string getString(const soci::row & row, const int32_t colId) {
        return row.get<std::string>(colId);
    }

    static Option<std::string> getOptionalString(const soci::row & row, const int32_t colId) {
        return row.get_indicator(colId) != soci::i_null ? Option<std::string>(row.get<std::string>(colId)) : Option<std::string>::NONE;
    }

    template<class Transformed>
    static Option<Transformed> getOptionalStringWithTransform(const soci::row & row, const int32_t colId, const std::function<Transformed(std::string)> & transform) {
        return row.get_indicator(colId) != soci::i_null ? Option<Transformed>(transform(row.get<std::string>(colId))) : Option<Transformed>::NONE;
    }

    static uint64_t getNumber(const soci::row & row, const int32_t colId) {
        return soci::get_number<uint64_t>(row, colId);
    }

    static Option<uint64_t> getOptionalNumber(const soci::row & row, const int32_t colId) {
        return row.get_indicator(colId) != soci::i_null ? Option<uint64_t>(soci::get_number<uint64_t>(row, colId)) : Option<uint64_t>::NONE;
    }

    template<class Transformed>
    static Option<Transformed> getOptionalNumberWithTransform(const soci::row & row, const int32_t colId, const std::function<Transformed(uint64_t)> & transform) {
        return row.get_indicator(colId) != soci::i_null ? Option<Transformed>(transform(soci::get_number<uint64_t>(row, colId))) : Option<Transformed>::NONE;
    }

    static const auto numToBool = [] (const uint64_t& num) { return !! num; };
    static const auto boolToNum = [] (const bool& b) { return static_cast<int32_t>(b); };
    static const auto stringToAddr = [] (const std::string& addr) { return Address(addr); };
    static const auto addrToString = [] (const Address& addr) { return addr.toString(); };
    static const auto b64toBytes = [] (const std::string& b64) {
        auto bytes = std::vector<uint8_t>();
        BaseConverter::decode(b64, BaseConverter::BASE64_RFC4648, bytes);
        return bytes;
    };
    static const auto bytesToB64 = [] (const std::vector<uint8_t>& bytes) {
        return BaseConverter::encode(bytes, BaseConverter::BASE64_RFC4648);
    };


    class TransactionDatabaseHelper {

    public:

        static bool transactionExists(soci::session & sql,
                                      const std::string & txUid);

        static bool getTransactionByHash(soci::session & sql,
                                         const std::string & hash,
                                         model::Transaction & tx);

        static std::string putTransaction(soci::session & sql,
                                          const std::string & accountUid,
                                          const model::Transaction & tx);

        static void deleteAllTransactions(soci::session& sql);

        static std::vector<model::Transaction> queryAssetTransferTransactionsInvolving(
                soci::session& sql,
                uint64_t assetId,
                const std::string& address);

        static std::vector<model::Transaction> queryTransactionsInvolving(
                soci::session& sql,
                const std::string& address);

        static void eraseDataSince(
                soci::session &sql,
                const std::string &accountUid,
                const std::chrono::system_clock::time_point & date);

    private:
        static std::vector<model::Transaction> query(
               const soci::rowset<soci::row>& rows);
    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDTRANSACTIONDATABASEHELPER_H
