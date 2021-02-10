/*
 *
 * CosmosLikeTransactionDatabaseHelper
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#ifndef LEDGER_CORE_COSMOSLIKETRANSACTIONDATABASEHELPER_H
#define LEDGER_CORE_COSMOSLIKETRANSACTIONDATABASEHELPER_H

#include <string>

#include <soci.h>
#include <wallet/cosmos/cosmos.hpp>

namespace ledger {
namespace core {

class CosmosLikeTransactionDatabaseHelper {
   public:
    static bool transactionExists(soci::session &sql, const std::string &cosmosTxUid);

    /// \brief DB --> Tx.
    static bool getTransactionByHash(
        soci::session &sql, const std::string &hash, cosmos::Transaction &tx);

    /// \brief DB --> Msg.
    static bool getMessageByUid(
        soci::session &sql, const std::string &msgUid, cosmos::Message &msg);

    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table.
    /// This function fills the common data, and then dispatch the message type specifics to
    /// another helper function.
    /// \param [in] sql The sql session to connect to the database (used when additional joins are
    /// necessary) \param [in] row The row from cosmos_messages table to use to fill the message
    /// data \param [out] msg The cosmos::Message to fill.
    static void inflateMessage(soci::session &sql, const soci::row &row, cosmos::Message &msg);

    static std::string createCosmosMessageUid(
        std::string const &txUid, 
        uint64_t msgIndex);

    static std::string createCosmosTransactionUid(
        std::string const &accountUid, 
        std::string const &txHash);

    /// \brief remove all operations and transactions from database since a given date
    static void eraseDataSince(
        soci::session &sql,
        const std::string &accountUid,
        const std::chrono::system_clock::time_point & date);

   private:
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgSend
    /// specific data. \param [in] row The row from cosmos_messages table to use \param [out] msg
    /// The cosmos::Message to fill
    static void inflateMsgSendSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgDelegate specific data. \param [in] row The row from cosmos_messages table to use \param
    /// [out] msg The cosmos::Message to fill
    static void inflateMsgDelegateSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgUndelegate specific data. \param [in] row The row from cosmos_messages table to use
    /// \param [out] msg The cosmos::Message to fill
    static void inflateMsgUndelegateSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgBeginRedelegate specific data. \param [in] row The row from cosmos_messages table to use
    /// \param [out] msg The cosmos::Message to fill
    static void inflateMsgBeginRedelegateSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgSubmitProposal specific data. \param [in] row The row from cosmos_messages table to use
    /// \param [out] msg The cosmos::Message to fill
    static void inflateMsgSubmitProposalSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgVote
    /// specific data. \param [in] row The row from cosmos_messages table to use \param [out] msg
    /// The cosmos::Message to fill
    static void inflateMsgVoteSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgDeposit
    /// specific data. \param [in] row The row from cosmos_messages table to use \param [out] msg
    /// The cosmos::Message to fill
    static void inflateMsgDepositSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgWithdrawDelegationReward specific data. \param [in] row The row from cosmos_messages
    /// table to use \param [out] msg The cosmos::Message to fill
    static void inflateMsgWithdrawDelegationRewardSpecifics(
        const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgWithdrawDelegatorReward specific data. \param [in] row The row from cosmos_messages table
    /// to use \param [out] msg The cosmos::Message to fill
    static void inflateMsgWithdrawDelegatorRewardSpecifics(
        const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgWithdrawValidatorCommission specific data. \param [in] row The row from cosmos_messages
    /// table to use \param [out] msg The cosmos::Message to fill
    static void inflateMsgWithdrawValidatorCommissionSpecifics(
        const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgSetWithdrawAddress specific data. \param [in] row The row from cosmos_messages table to
    /// use \param [out] msg The cosmos::Message to fill
    static void inflateMsgSetWithdrawAddressSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgUnjail
    /// specific data. \param [in] row The row from cosmos_messages table to use \param [out] msg
    /// The cosmos::Message to fill
    static void inflateMsgUnjailSpecifics(const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with
    /// MsgMultiSend specific data. \param [in] sql The sql session to connect to the database (used
    /// to join cosmos_multisend_io table) \param [in] row The row from cosmos_messages table to use
    /// \param [out] msg The cosmos::Message to fill
    static void inflateMsgMultiSendSpecifics(
        soci::session &sql, const soci::row &row, cosmos::Message &msg);
    /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgFees
    /// specific data. \param [in] row The row from cosmos_messages table to use \param [out] msg
    /// The cosmos::Message to fill
    static void inflateMsgFeesSpecifics(const soci::row &row, cosmos::Message &msg);

};
}  // namespace core
}  // namespace ledger

#endif  // LEDGER_CORE_COSMOSLIKETRANSACTIONDATABASEHELPER_H
